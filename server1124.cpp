#include "C:\Users\user\Desktop\3-2\��Ʈ��ũ���α׷���\Common.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512
#define MAX_CLIENTS 8

SOCKET clients[MAX_CLIENTS][2][2][2]; //Ŭ�󸮾�Ʈ ���ϰ� 0,1 �� �߾�� ����, �غ�Ϸ����, �켱���� ����
char server_nickname[MAX_CLIENTS][10]; // �г����� ������ ���� �ִ� 9�ڸ�
int GameOnOff[] = { 0, 0 }; //���ӽ��۽� 1
int ReadyCount = 0;

//�迭 ���
void print_array(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

//�迭 �������� �ű��(0�� �ƴѼ���)
void shift_nonzero_left(int arr[], int size) {
    int non_zero_index = 0;

    for (int i = 0; i < size; ++i) {
        if (arr[i] != 0) {
            arr[non_zero_index++] = arr[i];
        }
    }

    for (; non_zero_index < size; ++non_zero_index) {
        arr[non_zero_index] = 0;
    }
}

//�迭 ���� �� �ε��� ã��
int find_min_index(int arr[], int size) {
    if (size <= 0) {
        // ���� ó��: �迭�� ����ִ� ���
        return -1;
    }

    int min_index = 0;
    int min_value = arr[0];

    for (int i = 0; i < size; ++i) {
        if (arr[i] < min_value && arr[i] != 0) {
            min_value = arr[i];
            min_index = i;
        }
    }

    return min_index;
}

//�迭 ���� ��  ã��
int find_min_value(int arr[], int size) {
    if (size <= 0) {
        // ���� ó��: �迭�� ����ִ� ���
        return -1;
    }

    int min_value = arr[0];

    for (int i = 0; i < size; ++i) {
        if (arr[i] < min_value && arr[i] != 0) {
            min_value = arr[i];
        }
    }

    return min_value;
}

//Ŭ���̾�Ʈ �ʱ갪 ����
void init_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i][0][0][0] = INVALID_SOCKET; //0���� ���� ����
        clients[i][1][0][0] = 1; //i���� �߾�� ����
        clients[i][0][1][0] = 0; //i���� �غ�Ϸ���� ���� �����Ǿ��ִ»���
        clients[i][0][0][1] = 0; //�켱���� 0���� ����
    }
}

//������ �ε��� Ȯ��
int indexC(SOCKET client_sock) {
    int index = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i][0][0][0] == client_sock) {
            index = i;
            break;
        }
    }
    return index;
}

//Ư�� Ŭ���̾�Ʈ���Ը� send
void c_send(int client_index, const char* message) {
    if (client_index < 0 || client_index >= MAX_CLIENTS || clients[client_index][0][0][0] == INVALID_SOCKET) {
        printf("Invalid client index\n");
        return;
    }
    int retval = send(clients[client_index][0][0][0], message, strlen(message), 0);
    if (retval == SOCKET_ERROR) {
        err_display("c_send() Error");
    }
}

//���� Ŭ���̾�Ʈ �� Ȯ��
int connect_client() {
    int count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i][0][0][0] != INVALID_SOCKET) {
            count++; // ���� ����� Ŭ���̾�Ʈ�� ����
        }
    }
    return count; // ��� Ŭ���̾�Ʈ�� �����
}

//��� Ŭ���̾�Ʈ���� send (�����޽���)
void all_send(const char* message) {
    for (int i = 0; i < connect_client(); i++) {
        int retval = send(clients[i][0][0][0], message, strlen(message), 0);
        if (retval == SOCKET_ERROR) {
            err_display("all_send() Error");
        }
    }
}

//Ŭ���̾�Ʈ �߰�
void add_client(SOCKET client_sock) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i][0][0][0] == INVALID_SOCKET) {
            clients[i][0][0][0] = client_sock;
            if (i == 0) {
                c_send(indexC(client_sock), "YouIsFisrtClient\n");
                all_send("----------------------------------------\n");
            }
            break;
        }
    }
}

//��� Ŭ���̾�Ʈ ���� Ȯ��
bool check_all_clients_disconnected() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i][0][0][0] != INVALID_SOCKET) {
            return false; // ���� ����� Ŭ���̾�Ʈ�� ����
        }
    }
    return true; // ��� Ŭ���̾�Ʈ�� �����
}

//Ŭ���̾�Ʈ �ʱ�ȭ
void reset_client(int index) {
    closesocket(clients[index][0][0][0]); // ������ �ݰ�
    clients[index][0][0][0] = INVALID_SOCKET; // ������ INVALID_SOCKET���� �ʱ�ȭ
    clients[index][1][0][0] = 1;
    clients[index][0][1][0] = 0;
    clients[index][0][0][1] = 0;
}









//��� Ŭ���̾�Ʈ���� send (ä�ø޽���)
void a_send(const char* message, SOCKET client_sock) {
    char modified_message[1024];  // ����� ũ��� �迭�� ����
    strcpy(modified_message, message);  // ���� �޽����� ����
    strcat(modified_message, "\n");  // ���� ���ڸ� �߰�

    int C_index = indexC(client_sock);
    if (clients[C_index][1][0][0] == 1) {
        all_send(modified_message);
    }
}

//��� ready �ߴ��� Ȯ��
bool check_all_ready() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i][0][0][0] != INVALID_SOCKET && clients[i][0][1][0] == 0) {
            return false; // ���� �غ���� ���� Ŭ���̾�Ʈ�� ����
        }
    }
    return true; // ��� Ŭ���̾�Ʈ�� �غ��
}


//��ǥ���� ����
enum GameType {
    TWENTY = 1, //������
    PROBLEM_SOLVING, //����Ǯ��
    UP_AND_DOWN, //���ٿ�
    ENERGY
};

//���ݼ� ��ǥ
GameType getSelectedGame() {
    int VotesTWENTY = 0; //������ ��ǥ��
    int VotesQUIZ = 0; //����Ǯ�� ��ǥ��
    int VotesUD = 0; //���ٿ� ��ǥ��
    int VotesENERGY = 0; //������ ��ǥ��

    printf("getSelectedGame()�� ���� ��.\n");

    // �� Ŭ���̾�Ʈ�κ��� ��ǥ�� ����
    for (int i = 0; i < connect_client(); i++) {
        char voteBuf[BUFSIZE];
        char message[20] = "Please Vote\n";
        c_send(i, message);
        printf("Please Vote�� ����.\n");

        int bytesReceived = recv(clients[i][0][0][0], voteBuf, BUFSIZE, 0); //voteBuf�� ���ϵ����� ����
        if (bytesReceived > 0) {
            voteBuf[bytesReceived] = '\0'; // null ���� ���� �߰�
            printf("%s : %s\n", server_nickname[i], voteBuf);
            // ��ǥ ����� Ȯ���ϰ� ī��Ʈ
            int vote = atoi(voteBuf); //���ڿ� => ����
            switch (vote) {
            case TWENTY:
                VotesTWENTY++;
                printf("VotesTwenty: %d\n", VotesTWENTY);
                break;
            case PROBLEM_SOLVING:
                VotesQUIZ++;
                break;
            case UP_AND_DOWN:
                VotesUD++;
                break;
                // ���� ���� Ÿ���� �ִٸ� �߰� ����
            case ENERGY:
                VotesENERGY++;
                break;
            }
        }
        else {
            printf("��ǥ ����");
        }
        if (i == 0) {
            c_send(connect_client() - 1, "YOuiSLASTCliENT\n");
        }
    }

    // �ִ� ��ǥ�� ������ ����
    if (VotesTWENTY >= VotesQUIZ && VotesTWENTY >= VotesUD &&  VotesTWENTY>= VotesENERGY) {
        return TWENTY;
    }
    else if (VotesQUIZ >= VotesTWENTY && VotesQUIZ >= VotesUD && VotesQUIZ >= VotesENERGY) {
        return PROBLEM_SOLVING;
    }
    else if (VotesUD >= VotesTWENTY && VotesUD >= VotesQUIZ && VotesUD >= VotesENERGY) {
        return UP_AND_DOWN;
    }
    else {
        return ENERGY;
    }
}


//�켱���� ����
int* Priority_settings() {
    int random[MAX_CLIENTS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    int cnt = connect_client();
    int new_random;
    srand((unsigned)time(NULL));

    for (int i = 0; i < cnt; ++i) {

        new_random = (rand() % MAX_CLIENTS) + 1;
        random[i] = new_random;
        for (int j = 0; j < i; j++) {
            if (random[i] == random[j]) {
                i--;
            }
        }

    }

    int non_zero_count = 0;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (random[i] != 0) {
            clients[i][0][0][1] = random[i];
            non_zero_count++;
        }
        else {
            break;  // 0�� ������ �� �ڷδ� ��� 0�� ���̹Ƿ� Ž�� �ߴ�
        }
    }
    shift_nonzero_left(random, MAX_CLIENTS);
    printf("Generated random array: ");
    print_array(random, MAX_CLIENTS);
    printf("Number of non-zero elements: %d\n", non_zero_count);


    return random;
}

void all_mute() {
    all_send("MuTeOnChangeByServer\n");
}

void mute_cancel(int client_index) {
    clients[client_index][1][0][0] = 1;
    c_send(client_index, "MuTeOffChangeByServer\n");
}

void mute(int client_index) {
    clients[client_index][1][0][0] = 0;
    c_send(client_index, "MuTeOnChangeByServer\n");
}

//�г��Ӽ���
void setName_message(const char* message, SOCKET client_sock) {
    // ����: �޽����� "SET_NICKNAME"���� �����ϸ� �г��� ����
    int index = indexC(client_sock);

    if (strncmp(message, "SET_NICKNAME", 12) == 0) {
        // message���� �г����� �����ؼ� server_nickname�� ����
        strcpy(server_nickname[index], message + 13);
        char send_data[31];
        snprintf(send_data, sizeof(send_data), "NickNAMEis%s\n", server_nickname[index]);
        // ���� Ŭ���̾�Ʈ���� �ٽ� �ǵ����ֱ�
        c_send(index, send_data);
        c_send(index, "NickNameSETTING..!\n");
        all_send("\n");
        all_send("----------------------------------------\n");
        all_send("\n");
    }
}

int little_value_found(int array[], int m) {
    int smallest = 9; // ���� ���� �� �ʱ�ȭ 9�� �� ������ 8���ٴ� Ŀ�� �ȴ�.

    for (int i = 0; i < connect_client(); i++) {
        if (array[i] > m && smallest > array[i] && array[i] != 0) {
            smallest = array[i];
        }
    }

    return smallest;
}

int Find_index_by_value(int array[], int value) {
    int index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (array[i] == value) {
            index = i;
            break;
        }
    }
    return index;
}

int findSecondSmallestNonZero(int arr[], int size) {
    // �迭�� ����� ū�� Ȯ��
    if (size < 2) {
        printf("�迭�� ũ�Ⱑ ������� �ʽ��ϴ�.\n");
        return -1; // ������ ��Ÿ���� ���� ��ȯ
    }

    int smallest = INT_MAX; // ���� ū ���� ������ �ʱ�ȭ
    int secondSmallest = INT_MAX; // �� ��°�� ū ���� ������ �ʱ�ȭ

    for (int i = 0; i < size; i++) {
        if (arr[i] != 0) {
            if (arr[i] < smallest) {
                secondSmallest = smallest;
                smallest = arr[i];
            }
            else if (arr[i] < secondSmallest && arr[i] != smallest) {
                secondSmallest = arr[i];
            }
        }
    }

    // �� ��°�� ���� ���� ��ȯ
    if (secondSmallest == INT_MAX) {
        printf("0�� ������ ��ȿ�� ���� �����ϴ�.\n");
        return -1; // ��ȿ�� ���� ���� ��� ������ ��Ÿ���� ���� ��ȯ
    }
    else {
        return secondSmallest;
    }
}

//������
void play_twenty_questions() {

    int* random = Priority_settings(); //�켱���� �迭 ����
    int h_value = 0;
    int host_value;
    int smallest = find_min_value(random, MAX_CLIENTS);

    //������ ����, ������ �з�
    int TESTER = find_min_index(random, MAX_CLIENTS);
    int TESTER_value = find_min_value(random, MAX_CLIENTS);
    int PLAYER_ARRAY[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (random[i] != TESTER_value) {
            PLAYER_ARRAY[i] = random[i]; //PLAYER_ARRAY�� ������ �ƴ� ���� �ε����� ���� �ֱ�
        }
        else if (random[i] == TESTER_value) {
            PLAYER_ARRAY[i] = 0;
        }
    }
    print_array(PLAYER_ARRAY, MAX_CLIENTS);
    printf("TEST_client[%d]\n", TESTER);
    printf("TEST_value[%d]\n", TESTER_value);

    for (int i = 0; i < connect_client(); i++) {
        printf("server_nickname[%d] = %s\n", i, server_nickname[i]);
    }
    


    all_send("----------------------------------------\n");

    //�����ڿ��� �� �Է��϶�� send
    c_send(TESTER, "YOU IS TESTER. PLEASE ENTER THE ANSWER\n");

    for (int i = 0; i < connect_client(); i++) {
        if (i != TESTER) c_send(i, "TESTER IS TYPING...\n");
    }

    // �������� ���� �޾ƿ��� �ڵ� (����)
    char answerBuf[BUFSIZE];
    int bytesReceived = recv(clients[TESTER][0][0][0], answerBuf, BUFSIZE, 0);

    if (bytesReceived > 0) {
        answerBuf[bytesReceived] = '\0'; // null ���� ���� �߰�
        printf("�������� �� : %s\n", answerBuf);
        char TEST_data[100];
        snprintf(TEST_data, sizeof(TEST_data), "Tester chose the answer.\n\n----------------------------------------\n");
        all_send(TEST_data);
    }
    else {
        // ���� ���� ó��
        printf("answerBuf ���� ����\n");
    }

    // ������� �������� ������ �Է¹���
    //mute(TESTER);

    //������ 20�� �ݺ� ��ȯ
    for (int question_cnt = 0; question_cnt < 20; question_cnt++) {

        host_value = little_value_found(PLAYER_ARRAY, h_value);
        printf("%d ��° ��ȯ ==> host value = %d\n", question_cnt + 1, host_value);//ó�� ������ TESTER���� ū ���� ���� ��
        if (host_value == 9) {
            h_value = little_value_found(PLAYER_ARRAY, 0);
            printf("{host_value == 9}h_value = %d\n", h_value);
            host_value = h_value;
            printf("{host_value == 9}h_value = %d\n", host_value);
        }
        else {
            h_value = host_value;
            printf("%d ��° ��ȯ ==> h_value = %d\n", question_cnt + 1, h_value);
        }
        int host_index = Find_index_by_value(PLAYER_ARRAY, host_value); //i ���� ���� �� �ε��� ã��
        printf("%d ��° ��ȯ ==> host_index = %d\n", question_cnt + 1, host_index);


        char question_cnt_data[BUFSIZE];
        snprintf(question_cnt_data, sizeof(question_cnt_data), "%dth : [ %s ]\n", question_cnt + 1, server_nickname[host_index]);
        all_send(question_cnt_data);
        c_send(host_index, "YouIsHost!@!@.!@@!@\n");


        // �����ڿ��� ������ �޾ƿ´�
        char host_question[20][BUFSIZE];
        int question_recv = recv(clients[host_index][0][0][0], host_question[question_cnt], BUFSIZE, 0);

        if (question_recv > 0) {
            host_question[question_cnt][question_recv] = '\0'; // null ���� ���� �߰�
            printf("%d��° �������� ���� : %s\n", (question_cnt + 1), host_question[question_cnt]);
            char HOST_data[100];
            snprintf(HOST_data, sizeof(HOST_data), "{ %s }: %s\n", server_nickname[host_index], host_question[question_cnt]);
            all_send(HOST_data);

            //all_send(host_question[question_cnt]);
            if (strcmp(answerBuf, host_question[question_cnt]) == 0) {
                printf("������ %s. �½��ϴ�!.\n", answerBuf);
                char send_data[100];
                snprintf(send_data, sizeof(send_data), "~~{ %s }~~ That's correct!\n[ %s ]win .\n", host_question[question_cnt], server_nickname[host_index]);
                all_send(send_data);
                all_send("----------------------------------------\n");
                
                GameOnOff[1] = 0;
                ReadyCount = 0;
                for (int i = 0; i < connect_client(); i++) {
                    clients[i][0][1][0] = 0;
                }
                c_send(0, "GameOffYouCanReady\n");
                all_send("GameOVERrRRRrRRRRrR\n");
                break;
                // ���⿡ �����ϰ��� �ϴ� �ڵ带 �߰��� �� �ֽ��ϴ�.
            }
            else {
                printf("�� ���ڿ��� �ٸ��ϴ�.\n");
            }
        }
        else {
            // ���� ���� ó��
            printf("question ���� ����\n");
        }
        // ������� �������� ������ �Է¹���

        //TESTER���� YESNO â ����ϰ� ����
        c_send(TESTER, "YESNOYESNONOYESNOO\n");

        char TESTER_YESNO[BUFSIZE];
        int YESNO = recv(clients[TESTER][0][0][0], TESTER_YESNO, BUFSIZE, 0);
        if (YESNO > 0) {
            TESTER_YESNO[YESNO] = '\0'; // null ���� ���� �߰�
            printf("�������� ��� : %s\n", TESTER_YESNO);
            if (strcmp(TESTER_YESNO, "Yesssssssssssssssseses") == 0) {
                all_send("YES\n\n----------------------------------------\n");
            }
            else if (strcmp(TESTER_YESNO, "Nooooooooooooooooono") == 0) {
                all_send("NO\n\n----------------------------------------\n");
            }
        }
        else {
            // ���� ���� ó��
            printf("YESNO ���� ����\n");
        }
    }
    /*//���� ������ ����
    int a = little_value_found(PLAYER_ARRAY, 0);
    int b = Find_index_by_value(PLAYER_ARRAY, a);
    c_send(b, "YouIsHost!@!@.!@@!@");

    // �����ڿ��� ������ �޾ƿ´�
    char host_question[BUFSIZE];
    int question_recv = recv(clients[b][0][0][0], host_question, BUFSIZE, 0);

    if (question_recv > 0) {
        host_question[question_recv] = '\0'; // null ���� ���� �߰�
        printf("�������� ���� : %s\n", host_question);
        all_send(host_question);
    }
    else {
        // ���� ���� ó��
        printf("���� ����\n");
    }
    // ������� �������� ������ �Է¹���

    //TESTER���� YESNO â ����ϰ� ����
    c_send(TESTER, "YESNOYESNONOYESNOO");

    char TESTER_YESNO[BUFSIZE];
    int YESNO = recv(clients[TESTER][0][0][0], TESTER_YESNO, BUFSIZE, 0);
    if (YESNO > 0) {
        TESTER_YESNO[YESNO] = '\0'; // null ���� ���� �߰�
        printf("�������� ��� : %s\n", TESTER_YESNO);
        all_send(TESTER_YESNO);
    }
    else {
        // ���� ���� ó��
        printf("���� ����\n");
    }*/
}


//����Ǯ��
void solve_problems() {
    printf("Q");
}

//���ٿ�
void play_up_and_down() {
    printf("Up");
}

void energy_problems() {
    printf("E");
}
/*
void print_clients() {
    printf("Clients array:\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        printf("Client %d:\n", i + 1);
        printf("  Socket: %lld\n", clients[i][0][0][0]);
        printf("  �߾��: %d\n", clients[i][1][0][0]);
        printf("  �غ�Ϸ����: %d\n", clients[i][0][1][0]);
        printf("  �켱����: %d\n", clients[i][0][0][1]);
        printf("\n");
    }
}*/


void start_game() {
    //������ Ŭ���̾�Ʈ�� ������ ���Ŭ���̾�Ʈ �Է� 2���ϰ� �����
    printf("start_game()�� �����\n");

    GameOnOff[1] = 1;
    if (GameOnOff[1] == 1) all_mute();

    // ���� ���� 
    GameType selectedGame = getSelectedGame();

    // ���õ� ���ӿ� ���� ó��
    switch (selectedGame) {
    case TWENTY:
        // ������ ���� ���� ���� �߰�
        printf("������ ������ ���۵Ǿ����ϴ�!\n");
        play_twenty_questions();
        // ���⿡ ������ ������ ���� ������ �߰��ϼ���.
        break;
    case PROBLEM_SOLVING:
        // ����Ǯ�� ���� ���� ���� �߰�
        printf("����Ǯ�� ������ ���۵Ǿ����ϴ�!\n");
        solve_problems();
        // ���⿡ ����Ǯ�� ������ ���� ������ �߰��ϼ���.
        break;
    case UP_AND_DOWN:
        // ���ٿ� ���� ���� ���� �߰�
        printf("���ٿ� ������ ���۵Ǿ����ϴ�!\n");
        play_up_and_down();
        // ���⿡ ���ٿ� ������ ���� ������ �߰��ϼ���.
        break;
        // ���� ���� Ÿ���� �ִٸ� �߰� ����
    case ENERGY:
        // ������ ���� ���� ���� ���� �߰�
        printf("������ ���� ������ ���۵Ǿ����ϴ�!\n");
        energy_problems();
        // ���⿡ ���ٿ� ������ ���� ������ �߰��ϼ���.
        break;
         // ���� ���� Ÿ���� �ִٸ� �߰� ����
    }

        // ���� ���� �Ŀ��� Ŭ���̾�Ʈ�� ���¸� �ʱ�ȭ�ϰų� �ٸ� �۾��� ������ �� �ֽ��ϴ�.
        // ���� ���:
        // init_game();     
}

//�غ�Ϸ� ��ġ Ȯ���� set_ready
void setReady(const char* message, SOCKET client_sock) {
    int index = 0;

    if (strncmp(message, "SET_READY", 9) == 0) {
        index = indexC(client_sock);
        printf("index : %d\n", index);
        clients[index][0][1][0] = 1; //�غ�Ϸ�
        ReadyCount++;
        printf("{ %d }client --> Are you Ready?\n", ReadyCount);
        if (ReadyCount != connect_client()) {
            c_send(ReadyCount, "Are you Ready?\n");
           
        }
    }
    if (check_all_ready() && GameOnOff[1] == 0) {
        all_send("\n");
        all_send("----------------------------------------\n");
        
        for (int i = 0; i < connect_client() - 1; i++) {
            c_send(i, "NotLaSTCLientHERE\n");
        }
        start_game();
    }
}


DWORD WINAPI ProcessClient(LPVOID arg)
{
    int retval;
    SOCKET client_sock = (SOCKET)arg;
    struct sockaddr_in clientaddr;
    char addr[INET_ADDRSTRLEN];
    int addrlen;
    char buf[BUFSIZE + 1];

    // Ŭ���̾�Ʈ ���� ���
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
    inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

    while (1) {
        // ������ �ޱ�
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
            break;

        // ���� ������ ���
        buf[retval] = '\0';
        printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

        // ������ ������
        // ���� ������ ó�� �߰�
        if (strstr(buf, "####################") != NULL) {
            // Ŭ���̾�Ʈ���� ���� Ư�� ���ڿ��� ���Ե� ���
            // ���⿡�� �ʿ��� ó���� �߰��ϸ� �˴ϴ�.
            c_send(indexC(client_sock), "\n");
        }
        else if (strstr(buf, "SET_READY") != NULL) { //buf�� SET_READY�� �����ϰ� ������
            setReady(buf, client_sock);
        }
        else {
            a_send(buf, client_sock); //�߾���� ������ ���� ��ȭ����
        }


        // �޽��� ó��
        setName_message(buf, client_sock);

        /* ������ ������
        retval = send(client_sock, buf, retval, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }*/
        // Ŭ���̾�Ʈ�� ����� �� ���� �ʱ�ȭ
        if (retval == 0) {
            int client_index = indexC(client_sock);
            reset_client(client_index);
            break;
        }
    }

    // Ŭ���̾�Ʈ ���� �ʱ�ȭ
    int client_index = indexC(client_sock);
    reset_client(client_index);
    GameOnOff[1] = 0;
    // ���� �ݱ�
    closesocket(client_sock);
    printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
        addr, ntohs(clientaddr.sin_port));

    return 0;
}

int main(int argc, char* argv[])
{
    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // ���� ����
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // ������ ��ſ� ����� ����
    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    int addrlen;
    int len; //���� ���� ������
    char buf[BUFSIZE + 1]; //���� ���� ������
    HANDLE hThread;

    init_clients(); // Ŭ���̾�Ʈ �迭 �ʱ�ȭ

    while (1) {
        if (GameOnOff[1] != 1) {
            // accept()
            addrlen = sizeof(clientaddr);
            client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
            if (client_sock == INVALID_SOCKET) {
                err_display("accept()");
                break;
            }
        }
        // ������ Ŭ���̾�Ʈ ���� ���
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
        printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
            addr, ntohs(clientaddr.sin_port));

        // Ŭ���̾�Ʈ �߰�
        add_client(client_sock);

        // ������ ����
        hThread = CreateThread(NULL, 0, ProcessClient,
            (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) { closesocket(client_sock); }
        else { CloseHandle(hThread); }

    }

    // ���� �ݱ�
    closesocket(listen_sock);

    // ���� ����
    WSACleanup();
    return 0;
}
