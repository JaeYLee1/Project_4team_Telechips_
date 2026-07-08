#define _DEFAULT_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_UART_PORT   "/dev/ttyACM0"
#define UART_BAUDRATE       B115200

#define SOCKET_PATH         "/tmp/pbv_uart.sock"

#define MAX_CLIENTS         8
#define LINE_BUFFER_SIZE    192
#define CLIENT_BUFFER_SIZE  128
#define SOCKET_PACKET_SIZE  256

typedef struct
{
    int module_type;
    int auth_result;

    int weight_g;

    int current_temp_x10;
    int target_temp_x10;
    int peltier_duty;

    int motor_running;
    int motor_speed_level;
    int target_rpm_x10;
    int current_rpm_x10;
    int motor_duty_x10;

    int detect_state;
    int relay_state;
    int fsm_state;

} TelemetryData;

static volatile sig_atomic_t g_running = 1;

static char g_client_buffers[MAX_CLIENTS][CLIENT_BUFFER_SIZE];
static size_t g_client_indexes[MAX_CLIENTS];

static void signal_handler(int signal_number)
{
    (void)signal_number;
    g_running = 0;
}

static long long get_monotonic_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ((long long)ts.tv_sec * 1000LL)
           + (ts.tv_nsec / 1000000LL);
}

static int set_nonblocking(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);

    if (flags < 0)
    {
        return -1;
    }

    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static int write_all(int fd, const char *buffer, size_t length)
{
    size_t total_written = 0U;

    while (total_written < length)
    {
        ssize_t written_size;

        written_size = write(fd,
                             buffer + total_written,
                             length - total_written);

        if (written_size < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return -1;
        }

        total_written += (size_t)written_size;
    }

    return 0;
}

static int open_uart(const char *device_path)
{
    int fd;
    struct termios tty;

    fd = open(device_path, O_RDWR | O_NOCTTY | O_SYNC);

    if (fd < 0)
    {
        perror("UART open failed");
        return -1;
    }

    if (tcgetattr(fd, &tty) != 0)
    {
        perror("tcgetattr failed");
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, UART_BAUDRATE);
    cfsetispeed(&tty, UART_BAUDRATE);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~(IGNBRK | IXON | IXOFF | IXANY);
    tty.c_lflag = 0;
    tty.c_oflag = 0;

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        perror("tcsetattr failed");
        close(fd);
        return -1;
    }

    tcflush(fd, TCIOFLUSH);

    return fd;
}

static int create_unix_server(void)
{
    int server_fd;
    struct sockaddr_un address;

    unlink(SOCKET_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket create failed");
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;

    strncpy(address.sun_path,
            SOCKET_PATH,
            sizeof(address.sun_path) - 1U);

    if (bind(server_fd,
             (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("Socket bind failed");
        close(server_fd);
        return -1;
    }

    if (chmod(SOCKET_PATH, 0660) < 0)
    {
        perror("Socket chmod failed");
    }

    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("Socket listen failed");
        close(server_fd);
        unlink(SOCKET_PATH);
        return -1;
    }

    if (set_nonblocking(server_fd) < 0)
    {
        perror("Socket nonblocking setup failed");
        close(server_fd);
        unlink(SOCKET_PATH);
        return -1;
    }

    return server_fd;
}

/*
 * STM32 -> Pi UART Telemetry Format
 *
 * TEL,module,auth,weight,temp,target_temp,peltier,
 *     motor_run,motor_level,target_rpm,current_rpm,duty,
 *     detect,relay,fsm
 *
 * Example:
 * TEL,0,0,0,250,100,0,0,2,1200,0,0,1,0,1
 */
static bool parse_telemetry(const char *line,
                            TelemetryData *data)
{
    int parsed_count;

    parsed_count = sscanf(
        line,
        "TEL,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
        &data->module_type,
        &data->auth_result,
        &data->weight_g,
        &data->current_temp_x10,
        &data->target_temp_x10,
        &data->peltier_duty,
        &data->motor_running,
        &data->motor_speed_level,
        &data->target_rpm_x10,
        &data->current_rpm_x10,
        &data->motor_duty_x10,
        &data->detect_state,
        &data->relay_state,
        &data->fsm_state
    );

    return (parsed_count == 14);
}

static void print_telemetry(const TelemetryData *data,
                            long long elapsed_ms)
{
    printf("[UART RX] t=%lld ms | "
           "module=%d auth=%d weight=%d g "
           "temp=%.1f target_temp=%.1f peltier=%d | "
           "motor=%d level=%d target_rpm=%.1f "
           "current_rpm=%.1f duty=%.1f | "
           "detect=%d relay=%d fsm=%d\n",
           elapsed_ms,
           data->module_type,
           data->auth_result,
           data->weight_g,
           data->current_temp_x10 / 10.0,
           data->target_temp_x10 / 10.0,
           data->peltier_duty,
           data->motor_running,
           data->motor_speed_level,
           data->target_rpm_x10 / 10.0,
           data->current_rpm_x10 / 10.0,
           data->motor_duty_x10 / 10.0,
           data->detect_state,
           data->relay_state,
           data->fsm_state);

    fflush(stdout);
}

static void remove_client(int client_fds[], int index)
{
    if (client_fds[index] >= 0)
    {
        close(client_fds[index]);
        client_fds[index] = -1;
        g_client_indexes[index] = 0U;
    }
}

static void accept_new_clients(int server_fd,
                               int client_fds[])
{
    int client_fd;
    int i;

    while (1)
    {
        client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0)
        {
            if ((errno == EAGAIN) ||
                (errno == EWOULDBLOCK))
            {
                break;
            }

            perror("Socket accept failed");
            break;
        }

        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_fds[i] < 0)
            {
                client_fds[i] = client_fd;
                g_client_indexes[i] = 0U;

                printf("[IPC] GUI client connected.\n");
                fflush(stdout);

                client_fd = -1;
                break;
            }
        }

        if (client_fd >= 0)
        {
            printf("[IPC] Client limit reached.\n");
            close(client_fd);
        }
    }
}

static void send_to_client(int client_fd,
                           const char *packet)
{
    size_t length;

    if (client_fd < 0)
    {
        return;
    }

    length = strlen(packet);

    if (send(client_fd,
             packet,
             length,
             MSG_NOSIGNAL) < 0)
    {
        perror("Client send failed");
    }
}

static void broadcast_packet(const char *packet,
                             int client_fds[])
{
    int i;
    size_t packet_length;

    packet_length = strlen(packet);

    for (i = 0; i < MAX_CLIENTS; i++)
    {
        ssize_t sent_size;

        if (client_fds[i] < 0)
        {
            continue;
        }

        sent_size = send(client_fds[i],
                         packet,
                         packet_length,
                         MSG_NOSIGNAL);

        if ((sent_size <= 0) &&
            (errno != EAGAIN) &&
            (errno != EWOULDBLOCK))
        {
            printf("[IPC] GUI client disconnected.\n");
            remove_client(client_fds, i);
        }
    }
}

static bool is_valid_gui_command_char(char command)
{
    switch (command)
    {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case 'c':
        case 'C':
        case 'd':
        case 'D':
        case 'r':
        case 'R':
            return true;

        default:
            return false;
    }
}

static void process_gui_command(int client_index,
                                int uart_fd,
                                int client_fds[],
                                const char *line)
{
    const char *command_text;
    char command;
    char response_packet[64];

    if (strncmp(line, "CMD,", 4U) != 0)
    {
        send_to_client(client_fds[client_index],
                       "SVC,ERR,INVALID_PROTOCOL\n");
        return;
    }

    command_text = line + 4;

    /*
     * GUI -> Service : CMD,c\n
     * Service -> STM32: c
     *
     * STM32 입장에서는 한 글자 명령만 받음.
     * Enter 필요 없음.
     */
    if (strlen(command_text) != 1U)
    {
        send_to_client(client_fds[client_index],
                       "SVC,ERR,INVALID_COMMAND\n");
        return;
    }

    command = command_text[0];

    if (!is_valid_gui_command_char(command))
    {
        send_to_client(client_fds[client_index],
                       "SVC,ERR,INVALID_COMMAND\n");
        return;
    }

    if (write_all(uart_fd, &command, 1U) != 0)
    {
        perror("UART write failed");

        send_to_client(client_fds[client_index],
                       "SVC,ERR,UART_WRITE\n");
        return;
    }

    printf("[GUI CMD] STM32 TX : %c\n", command);
    fflush(stdout);

    snprintf(response_packet,
             sizeof(response_packet),
             "SVC,CMD_SENT,%c\n",
             command);

    send_to_client(client_fds[client_index],
                   response_packet);
}

static void process_client_data(int client_index,
                                int uart_fd,
                                int client_fds[])
{
    char receive_buffer[64];
    ssize_t received_size;
    ssize_t i;

    received_size = recv(client_fds[client_index],
                         receive_buffer,
                         sizeof(receive_buffer),
                         0);

    if (received_size <= 0)
    {
        printf("[IPC] GUI client disconnected.\n");
        remove_client(client_fds, client_index);
        return;
    }

    for (i = 0; i < received_size; i++)
    {
        char received_char = receive_buffer[i];

        if (received_char == '\r')
        {
            continue;
        }

        if (received_char == '\n')
        {
            if (g_client_indexes[client_index] > 0U)
            {
                g_client_buffers[client_index]
                                [g_client_indexes[client_index]] = '\0';

                process_gui_command(client_index,
                                    uart_fd,
                                    client_fds,
                                    g_client_buffers[client_index]);

                g_client_indexes[client_index] = 0U;
            }

            continue;
        }

        if (g_client_indexes[client_index] <
            (CLIENT_BUFFER_SIZE - 1U))
        {
            g_client_buffers[client_index]
                            [g_client_indexes[client_index]] =
                received_char;

            g_client_indexes[client_index]++;
        }
        else
        {
            printf("[IPC] GUI command buffer overflow.\n");
            g_client_indexes[client_index] = 0U;
        }
    }
}

int main(int argc, char *argv[])
{
    const char *uart_port = DEFAULT_UART_PORT;

    int uart_fd;
    int server_fd;
    int client_fds[MAX_CLIENTS];

    char line_buffer[LINE_BUFFER_SIZE];
    size_t line_index = 0U;

    char received_char;
    char socket_packet[SOCKET_PACKET_SIZE];

    TelemetryData telemetry;
    long long start_ms;

    fd_set read_fds;
    int max_fd;
    int select_result;
    int i;

    if (argc >= 2)
    {
        uart_port = argv[1];
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    for (i = 0; i < MAX_CLIENTS; i++)
    {
        client_fds[i] = -1;
        g_client_indexes[i] = 0U;
    }

    printf("========================================\n");
    printf(" PBV UART Service\n");
    printf(" UART Port   : %s\n", uart_port);
    printf(" Baud Rate   : 115200\n");
    printf(" Socket Path : %s\n", SOCKET_PATH);
    printf("========================================\n");

    uart_fd = open_uart(uart_port);

    if (uart_fd < 0)
    {
        return EXIT_FAILURE;
    }

    server_fd = create_unix_server();

    if (server_fd < 0)
    {
        close(uart_fd);
        return EXIT_FAILURE;
    }

    printf("UART connected. Waiting for TEL packets...\n");

    start_ms = get_monotonic_ms();

    while (g_running)
    {
        FD_ZERO(&read_fds);

        FD_SET(uart_fd, &read_fds);
        FD_SET(server_fd, &read_fds);

        max_fd = (uart_fd > server_fd)
                     ? uart_fd
                     : server_fd;

        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_fds[i] >= 0)
            {
                FD_SET(client_fds[i], &read_fds);

                if (client_fds[i] > max_fd)
                {
                    max_fd = client_fds[i];
                }
            }
        }

        select_result = select(max_fd + 1,
                               &read_fds,
                               NULL,
                               NULL,
                               NULL);

        if (select_result < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            perror("select failed");
            break;
        }

        if (FD_ISSET(server_fd, &read_fds))
        {
            accept_new_clients(server_fd, client_fds);
        }

        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if ((client_fds[i] >= 0) &&
                FD_ISSET(client_fds[i], &read_fds))
            {
                process_client_data(i,
                                    uart_fd,
                                    client_fds);
            }
        }

        if (!FD_ISSET(uart_fd, &read_fds))
        {
            continue;
        }

        if (read(uart_fd, &received_char, 1) != 1)
        {
            continue;
        }

        if (received_char == '\r')
        {
            continue;
        }

        if (received_char == '\n')
        {
            line_buffer[line_index] = '\0';

            if (line_index > 0U)
            {
                if (parse_telemetry(line_buffer, &telemetry))
                {
                    print_telemetry(&telemetry,
                                    get_monotonic_ms() - start_ms);

                    snprintf(
                        socket_packet,
                        sizeof(socket_packet),
                        "TEL,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                        telemetry.module_type,
                        telemetry.auth_result,
                        telemetry.weight_g,
                        telemetry.current_temp_x10,
                        telemetry.target_temp_x10,
                        telemetry.peltier_duty,
                        telemetry.motor_running,
                        telemetry.motor_speed_level,
                        telemetry.target_rpm_x10,
                        telemetry.current_rpm_x10,
                        telemetry.motor_duty_x10,
                        telemetry.detect_state,
                        telemetry.relay_state,
                        telemetry.fsm_state
                    );

                    broadcast_packet(socket_packet,
                                     client_fds);
                }
                else if ((strncmp(line_buffer, "OK,", 3U) == 0) ||
                         (strncmp(line_buffer, "ERR,", 4U) == 0))
                {
                    snprintf(socket_packet,
                             sizeof(socket_packet),
                             "%s\n",
                             line_buffer);

                    printf("[STM32 ACK] %s\n", line_buffer);
                    fflush(stdout);

                    broadcast_packet(socket_packet,
                                     client_fds);
                }
                else
                {
                    printf("[UART RX] Unknown line: %s\n",
                           line_buffer);
                    fflush(stdout);
                }
            }

            line_index = 0U;
            continue;
        }

        if (line_index < (LINE_BUFFER_SIZE - 1U))
        {
            line_buffer[line_index++] = received_char;
        }
        else
        {
            printf("[UART RX] Line buffer overflow.\n");
            line_index = 0U;
        }
    }

    for (i = 0; i < MAX_CLIENTS; i++)
    {
        remove_client(client_fds, i);
    }

    close(server_fd);
    close(uart_fd);

    unlink(SOCKET_PATH);

    printf("\nUART service stopped.\n");

    return EXIT_SUCCESS;
}