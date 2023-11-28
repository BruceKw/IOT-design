#include "C:\Users\user\Desktop\3-2\네트워크프로그래밍\Common.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512
#define MAX_CLIENTS 8

SOCKET clients[MAX_CLIENTS][2][2][2]; //클라리언트 소켓과 0,1 로 발언권 설정, 준비완료상태, 우선순위 설정
char server_nickname[MAX_CLIENTS][10]; // 닉네임을 저장할 변수 최대 9자리
int GameOnOff[] = { 0, 0 }; //게임시작시 1
int ReadyCount = 0;

//배열 출력
void print_array(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

//배열 왼쪽으로 옮기기(0이 아닌수만)
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

//배열 작은 수 인덱스 찾기
int find_min_index(int arr[], int size) {
    if (size <= 0) {
        // 예외 처리: 배열이 비어있는 경우
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

//배열 작은 수  찾기
int find_min_value(int arr[], int size) {
    if (size <= 0) {
        // 예외 처리: 배열이 비어있는 경우
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

//클라이언트 초깃값 선언
void init_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i][0][0][0] = INVALID_SOCKET; //0열에 소켓 저장
        clients[i][1][0][0] = 1; //i열에 발언권 설정
        clients[i][0][1][0] = 0; //i열에 준비완료상태 설정 해제되어있는상태
        clients[i][0][0][1] = 0; //우선순위 0으로 설정
    }
}

//소켓의 인덱스 확인
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

//특정 클라이언트에게만 send
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

//접속 클라이언트 수 확인
int connect_client() {
    int count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i][0][0][0] != INVALID_SOCKET) {
            count++; // 아직 연결된 클라이언트가 있음
        }
    }
    return count; // 모든 클라이언트가 종료됨
}

//모든 클라이언트에게 send (공지메시지)
void all_send(const char* message) {
    for (int i = 0; i < connect_client(); i++) {
        int retval = send(clients[i][0][0][0], message, strlen(message), 0);
        if (retval == SOCKET_ERROR) {
            err_display("all_send() Error");
        }
    }
}

//클라이언트 추가
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

//모든 클라이언트 종료 확인
bool check_all_clients_disconnected() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i][0][0][0] != INVALID_SOCKET) {
            return false; // 아직 연결된 클라이언트가 있음
        }
    }
    return true; // 모든 클라이언트가 종료됨
}

//클라이언트 초기화
void reset_client(int index) {
    closesocket(clients[index][0][0][0]); // 소켓을 닫고
    clients[index][0][0][0] = INVALID_SOCKET; // 소켓을 INVALID_SOCKET으로 초기화
    clients[index][1][0][0] = 1;
    clients[index][0][1][0] = 0;
    clients[index][0][0][1] = 0;
}









//모든 클라이언트에게 send (채팅메시지)
void a_send(const char* message, SOCKET client_sock) {
    char modified_message[1024];  // 충분한 크기로 배열을 선언
    strcpy(modified_message, message);  // 원본 메시지를 복사
    strcat(modified_message, "\n");  // 개행 문자를 추가

    int C_index = indexC(client_sock);
    if (clients[C_index][1][0][0] == 1) {
        all_send(modified_message);
    }
}

//모두 ready 했는지 확인
bool check_all_ready() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i][0][0][0] != INVALID_SOCKET && clients[i][0][1][0] == 0) {
            return false; // 아직 준비되지 않은 클라이언트가 있음
        }
    }
    return true; // 모든 클라이언트가 준비됨
}


//투표받을 게임
enum GameType {
    TWENTY = 1, //스무고개
    PROBLEM_SOLVING, //문제풀이
    UP_AND_DOWN, //업다운
    ENERGY
};

//과반수 투표
GameType getSelectedGame() {
    int VotesTWENTY = 0; //스무고개 투표수
    int VotesQUIZ = 0; //문제풀이 투표수
    int VotesUD = 0; //업다운 투표수
    int VotesENERGY = 0; //에너지 투표수

    printf("getSelectedGame()이 실행 됨.\n");

    // 각 클라이언트로부터 투표를 받음
    for (int i = 0; i < connect_client(); i++) {
        char voteBuf[BUFSIZE];
        char message[20] = "Please Vote\n";
        c_send(i, message);
        printf("Please Vote를 보냄.\n");

        int bytesReceived = recv(clients[i][0][0][0], voteBuf, BUFSIZE, 0); //voteBuf에 소켓데이터 저장
        if (bytesReceived > 0) {
            voteBuf[bytesReceived] = '\0'; // null 종료 문자 추가
            printf("%s : %s\n", server_nickname[i], voteBuf);
            // 투표 결과를 확인하고 카운트
            int vote = atoi(voteBuf); //문자열 => 정수
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
                // 여러 게임 타입이 있다면 추가 가능
            case ENERGY:
                VotesENERGY++;
                break;
            }
        }
        else {
            printf("투표 오류");
        }
        if (i == 0) {
            c_send(connect_client() - 1, "YOuiSLASTCliENT\n");
        }
    }

    // 최다 투표된 게임을 선택
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


//우선순위 설정
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
            break;  // 0이 나오면 그 뒤로는 모두 0일 것이므로 탐색 중단
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

//닉네임설정
void setName_message(const char* message, SOCKET client_sock) {
    // 예시: 메시지가 "SET_NICKNAME"으로 시작하면 닉네임 설정
    int index = indexC(client_sock);

    if (strncmp(message, "SET_NICKNAME", 12) == 0) {
        // message에서 닉네임을 추출해서 server_nickname에 저장
        strcpy(server_nickname[index], message + 13);
        char send_data[31];
        snprintf(send_data, sizeof(send_data), "NickNAMEis%s\n", server_nickname[index]);
        // 보낸 클라이언트에게 다시 되돌려주기
        c_send(index, send_data);
        c_send(index, "NickNameSETTING..!\n");
        all_send("\n");
        all_send("----------------------------------------\n");
        all_send("\n");
    }
}

int little_value_found(int array[], int m) {
    int smallest = 9; // 가장 작은 값 초기화 9로 한 이유는 8보다는 커야 된다.

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
    // 배열이 충분히 큰지 확인
    if (size < 2) {
        printf("배열의 크기가 충분하지 않습니다.\n");
        return -1; // 에러를 나타내는 값을 반환
    }

    int smallest = INT_MAX; // 가장 큰 정수 값으로 초기화
    int secondSmallest = INT_MAX; // 두 번째로 큰 정수 값으로 초기화

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

    // 두 번째로 작은 값을 반환
    if (secondSmallest == INT_MAX) {
        printf("0을 제외한 유효한 값이 없습니다.\n");
        return -1; // 유효한 값이 없는 경우 에러를 나타내는 값을 반환
    }
    else {
        return secondSmallest;
    }
}

//스무고개
void play_twenty_questions() {

    int* random = Priority_settings(); //우선순위 배열 설정
    int h_value = 0;
    int host_value;
    int smallest = find_min_value(random, MAX_CLIENTS);

    //출제자 선정, 제출자 분류
    int TESTER = find_min_index(random, MAX_CLIENTS);
    int TESTER_value = find_min_value(random, MAX_CLIENTS);
    int PLAYER_ARRAY[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (random[i] != TESTER_value) {
            PLAYER_ARRAY[i] = random[i]; //PLAYER_ARRAY에 출제자 아닌 값만 인덱스에 맞춰 넣기
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

    //출제자에게 답 입력하라고 send
    c_send(TESTER, "YOU IS TESTER. PLEASE ENTER THE ANSWER\n");

    for (int i = 0; i < connect_client(); i++) {
        if (i != TESTER) c_send(i, "TESTER IS TYPING...\n");
    }

    // 출제자의 답을 받아오는 코드 (가정)
    char answerBuf[BUFSIZE];
    int bytesReceived = recv(clients[TESTER][0][0][0], answerBuf, BUFSIZE, 0);

    if (bytesReceived > 0) {
        answerBuf[bytesReceived] = '\0'; // null 종료 문자 추가
        printf("출제자의 답 : %s\n", answerBuf);
        char TEST_data[100];
        snprintf(TEST_data, sizeof(TEST_data), "Tester chose the answer.\n\n----------------------------------------\n");
        all_send(TEST_data);
    }
    else {
        // 수신 에러 처리
        printf("answerBuf 수신 에러\n");
    }

    // 여기까지 출제자의 정답을 입력받음
    //mute(TESTER);

    //제출자 20번 반복 순환
    for (int question_cnt = 0; question_cnt < 20; question_cnt++) {

        host_value = little_value_found(PLAYER_ARRAY, h_value);
        printf("%d 번째 순환 ==> host value = %d\n", question_cnt + 1, host_value);//처음 시작은 TESTER보다 큰 가장 작은 수
        if (host_value == 9) {
            h_value = little_value_found(PLAYER_ARRAY, 0);
            printf("{host_value == 9}h_value = %d\n", h_value);
            host_value = h_value;
            printf("{host_value == 9}h_value = %d\n", host_value);
        }
        else {
            h_value = host_value;
            printf("%d 번째 순환 ==> h_value = %d\n", question_cnt + 1, h_value);
        }
        int host_index = Find_index_by_value(PLAYER_ARRAY, host_value); //i 보다 작은 값 인덱스 찾기
        printf("%d 번째 순환 ==> host_index = %d\n", question_cnt + 1, host_index);


        char question_cnt_data[BUFSIZE];
        snprintf(question_cnt_data, sizeof(question_cnt_data), "%dth : [ %s ]\n", question_cnt + 1, server_nickname[host_index]);
        all_send(question_cnt_data);
        c_send(host_index, "YouIsHost!@!@.!@@!@\n");


        // 제출자에게 질문을 받아온다
        char host_question[20][BUFSIZE];
        int question_recv = recv(clients[host_index][0][0][0], host_question[question_cnt], BUFSIZE, 0);

        if (question_recv > 0) {
            host_question[question_cnt][question_recv] = '\0'; // null 종료 문자 추가
            printf("%d번째 제출자의 질문 : %s\n", (question_cnt + 1), host_question[question_cnt]);
            char HOST_data[100];
            snprintf(HOST_data, sizeof(HOST_data), "{ %s }: %s\n", server_nickname[host_index], host_question[question_cnt]);
            all_send(HOST_data);

            //all_send(host_question[question_cnt]);
            if (strcmp(answerBuf, host_question[question_cnt]) == 0) {
                printf("정답은 %s. 맞습니다!.\n", answerBuf);
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
                // 여기에 실행하고자 하는 코드를 추가할 수 있습니다.
            }
            else {
                printf("두 문자열은 다릅니다.\n");
            }
        }
        else {
            // 수신 에러 처리
            printf("question 수신 에러\n");
        }
        // 여기까지 제출자의 질문을 입력받음

        //TESTER에게 YESNO 창 출력하게 만듬
        c_send(TESTER, "YESNOYESNONOYESNOO\n");

        char TESTER_YESNO[BUFSIZE];
        int YESNO = recv(clients[TESTER][0][0][0], TESTER_YESNO, BUFSIZE, 0);
        if (YESNO > 0) {
            TESTER_YESNO[YESNO] = '\0'; // null 종료 문자 추가
            printf("출제자의 대답 : %s\n", TESTER_YESNO);
            if (strcmp(TESTER_YESNO, "Yesssssssssssssssseses") == 0) {
                all_send("YES\n\n----------------------------------------\n");
            }
            else if (strcmp(TESTER_YESNO, "Nooooooooooooooooono") == 0) {
                all_send("NO\n\n----------------------------------------\n");
            }
        }
        else {
            // 수신 에러 처리
            printf("YESNO 수신 에러\n");
        }
    }
    /*//다음 제출자 선정
    int a = little_value_found(PLAYER_ARRAY, 0);
    int b = Find_index_by_value(PLAYER_ARRAY, a);
    c_send(b, "YouIsHost!@!@.!@@!@");

    // 제출자에게 질문을 받아온다
    char host_question[BUFSIZE];
    int question_recv = recv(clients[b][0][0][0], host_question, BUFSIZE, 0);

    if (question_recv > 0) {
        host_question[question_recv] = '\0'; // null 종료 문자 추가
        printf("제출자의 질문 : %s\n", host_question);
        all_send(host_question);
    }
    else {
        // 수신 에러 처리
        printf("수신 에러\n");
    }
    // 여기까지 제출자의 질문을 입력받음

    //TESTER에게 YESNO 창 출력하게 만듬
    c_send(TESTER, "YESNOYESNONOYESNOO");

    char TESTER_YESNO[BUFSIZE];
    int YESNO = recv(clients[TESTER][0][0][0], TESTER_YESNO, BUFSIZE, 0);
    if (YESNO > 0) {
        TESTER_YESNO[YESNO] = '\0'; // null 종료 문자 추가
        printf("출제자의 대답 : %s\n", TESTER_YESNO);
        all_send(TESTER_YESNO);
    }
    else {
        // 수신 에러 처리
        printf("수신 에러\n");
    }*/
}


//문제풀이
void solve_problems() {
    printf("Q");
}

//업다운
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
        printf("  발언권: %d\n", clients[i][1][0][0]);
        printf("  준비완료상태: %d\n", clients[i][0][1][0]);
        printf("  우선순위: %d\n", clients[i][0][0][1]);
        printf("\n");
    }
}*/


void start_game() {
    //마지막 클라이언트를 제외한 모든클라이언트 입력 2번하게 만들기
    printf("start_game()이 실행됨\n");

    GameOnOff[1] = 1;
    if (GameOnOff[1] == 1) all_mute();

    // 게임 선택 
    GameType selectedGame = getSelectedGame();

    // 선택된 게임에 따라 처리
    switch (selectedGame) {
    case TWENTY:
        // 스무고개 게임 시작 로직 추가
        printf("스무고개 게임이 시작되었습니다!\n");
        play_twenty_questions();
        // 여기에 스무고개 게임의 세부 동작을 추가하세요.
        break;
    case PROBLEM_SOLVING:
        // 문제풀이 게임 시작 로직 추가
        printf("문제풀이 게임이 시작되었습니다!\n");
        solve_problems();
        // 여기에 문제풀이 게임의 세부 동작을 추가하세요.
        break;
    case UP_AND_DOWN:
        // 업다운 게임 시작 로직 추가
        printf("업다운 게임이 시작되었습니다!\n");
        play_up_and_down();
        // 여기에 업다운 게임의 세부 동작을 추가하세요.
        break;
        // 여러 게임 타입이 있다면 추가 가능
    case ENERGY:
        // 에너지 관련 게임 시작 로직 추가
        printf("에너지 문제 게임이 시작되었습니다!\n");
        energy_problems();
        // 여기에 업다운 게임의 세부 동작을 추가하세요.
        break;
         // 여러 게임 타입이 있다면 추가 가능
    }

        // 게임 시작 후에는 클라이언트의 상태를 초기화하거나 다른 작업을 수행할 수 있습니다.
        // 예를 들어:
        // init_game();     
}

//준비완료 위치 확인후 set_ready
void setReady(const char* message, SOCKET client_sock) {
    int index = 0;

    if (strncmp(message, "SET_READY", 9) == 0) {
        index = indexC(client_sock);
        printf("index : %d\n", index);
        clients[index][0][1][0] = 1; //준비완료
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

    // 클라이언트 정보 얻기
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
    inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

    while (1) {
        // 데이터 받기
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
            break;

        // 받은 데이터 출력
        buf[retval] = '\0';
        printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

        // 데이터 보내기
        // 수신 데이터 처리 추가
        if (strstr(buf, "####################") != NULL) {
            // 클라이언트에서 보낸 특정 문자열이 포함된 경우
            // 여기에서 필요한 처리를 추가하면 됩니다.
            c_send(indexC(client_sock), "\n");
        }
        else if (strstr(buf, "SET_READY") != NULL) { //buf가 SET_READY를 포함하고 있으면
            setReady(buf, client_sock);
        }
        else {
            a_send(buf, client_sock); //발언권이 있으면 단톡 대화가능
        }


        // 메시지 처리
        setName_message(buf, client_sock);

        /* 데이터 보내기
        retval = send(client_sock, buf, retval, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }*/
        // 클라이언트가 종료될 때 소켓 초기화
        if (retval == 0) {
            int client_index = indexC(client_sock);
            reset_client(client_index);
            break;
        }
    }

    // 클라이언트 정보 초기화
    int client_index = indexC(client_sock);
    reset_client(client_index);
    GameOnOff[1] = 0;
    // 소켓 닫기
    closesocket(client_sock);
    printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
        addr, ntohs(clientaddr.sin_port));

    return 0;
}

int main(int argc, char* argv[])
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // 소켓 생성
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

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    int addrlen;
    int len; //고정 길이 데이터
    char buf[BUFSIZE + 1]; //가변 길이 데이터
    HANDLE hThread;

    init_clients(); // 클라이언트 배열 초기화

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
        // 접속한 클라이언트 정보 출력
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
            addr, ntohs(clientaddr.sin_port));

        // 클라이언트 추가
        add_client(client_sock);

        // 스레드 생성
        hThread = CreateThread(NULL, 0, ProcessClient,
            (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) { closesocket(client_sock); }
        else { CloseHandle(hThread); }

    }

    // 소켓 닫기
    closesocket(listen_sock);

    // 윈속 종료
    WSACleanup();
    return 0;
}
