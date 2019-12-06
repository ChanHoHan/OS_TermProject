// 2015270209_한찬호_ROUND_ROBIN_구현
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<stdlib.h>
#include<time.h>


#define BUFSIZE 1024


int physical_memory[80][4] = { 0, }; // 총80프레임  한 프레임당 4byte로 생각. 
int physical_memory_process_and_LRU[80][2] = { 0, };  // 피지컬 메모리에 어떤 프로세스가 있고, 사용하지 않은 시간 기록.  [80][0]은 process번호 [80][1]은 시간



struct Logical {
	int pagetable[11][2]; // 페이지 테이블
	int logical_memory[11][4];// 총 11 page   한 페이지당 4byte로 생각.
};


struct PCB {
	HANDLE PWhandle; //parent handle
	HANDLE PRhandle; //child handle
	int cpu_burst;
	int io_burst;
	int pid;
	int p_number; // process 번호 
	struct Logical logical;
};




///////////// ready queue 구현 ////////////
int readyqueue[11] = { 0, };
int front, rear;

int init_readyqueue(void) {
	front = rear = 0;
}

void readyqueue_put(int k) {
	if ((rear + 1) % 11 == front) {
		printf("\n overflow");
		return -1;
	}
	readyqueue[rear] = k;
	rear = ++rear % 11;
}

int readyqueue_get() {
	int i;
	if (front == rear) {
		printf("\n underflow");
		return -1;
	}

	i = readyqueue[front];
	front = ++front % 11;
	return i;
}
/////////////////////////////////


int _tmain(int argc, TCHAR* argv[]) {
	
	memset(physical_memory_process_and_LRU, -1, sizeof(physical_memory_process_and_LRU));

	srand(time(NULL)); // 난수
	int random = 0;


	FILE *fp = fopen("memory_management_dump.txt", "w");

	
	//구조체 포인터 배열 사용
	struct PCB *p[11];
	for (int i = 0; i < sizeof(p) / sizeof(struct PCB *); i++) {
		p[i] = malloc(sizeof(struct PCB));
	}

	p[0]->cpu_burst = 10; p[0]->io_burst = 5;
	p[1]->cpu_burst = 7; p[1]->io_burst = 5;
	p[2]->cpu_burst = 10; p[2]->io_burst = 5;
	p[3]->cpu_burst = 10; p[3]->io_burst = 5;
	p[4]->cpu_burst = 4; p[4]->io_burst = 5;
	p[5]->cpu_burst = 10; p[5]->io_burst = 5;
	p[6]->cpu_burst = 3; p[6]->io_burst = 5;
	p[7]->cpu_burst = 10; p[7]->io_burst = 5;
	p[8]->cpu_burst = 1; p[8]->io_burst = 5;
	p[9]->cpu_burst = 10; p[9]->io_burst = 5;



	//이름없는 파이프
	HANDLE hReadPipe = NULL, hWritePipe = NULL;
	TCHAR recvString[100];
	DWORD bytesWritten;
	DWORD bytesRead;

	STARTUPINFO si = { 0, };
	PROCESS_INFORMATION pi;
	//si.cb = sizeof(si);


	//// 2차 텀 수정 ////
	HANDLE YourRPipe = NULL;
	HANDLE YourWPipe = NULL;

	 
	si.hStdError = hWritePipe;
	si.hStdOutput = hWritePipe;
	si.hStdInput = YourRPipe;
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;


	//////////////////////
	DWORD dwRead1;
	TCHAR chBuf2[4096] = { 0, };

	int head = 0, tail = 0;

	init_readyqueue();

	for (int i = 0; i < 10; i++) {  // 자식 프로세스 10개 생성

		// 2차 텀프 //
		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;
		//


		/* 나의 pipe 생성 */
		if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 4096)) {
			printf("create pipe error\n");
			return -1;
		}

		//// 2차 텀프 /////

		if (!CreatePipe(&YourRPipe, &YourWPipe, &saAttr, 4096)) {
			printf("create pipe error\n");
			return -1;
		}

		SetHandleInformation(YourRPipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

		/* pipe의 다른 한쪽 끝을 이용한 데이터 송신*/
		TCHAR cmdString[4096] = { 0, };
		SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		wsprintf(cmdString, _T("%s %u %u"), _T("childprocess.exe"), hReadPipe, YourWPipe);


		CreateProcess(NULL, cmdString, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);


		//write 하는 부분//
		char data[4096] = "hi";
		//_itoa(p[i]->cpu_burst, data, 10);

		printf("string send: %s \n", data);
		WriteFile(hWritePipe, data, 4096, &bytesWritten, NULL);  // 자식 프로세스에게 값 전달
		printf("handle : %d\n", hReadPipe);

		DWORD dwRead;
		TCHAR chBuf[4096] = { 0, };

		ReadFile(YourRPipe, chBuf, 4096, &dwRead, NULL);

		printf("read file : %s\n", chBuf);


		char data1[4096] = "who are you";
		WriteFile(hWritePipe, data1, 4096, &bytesWritten, NULL);  // 자식 프로세스에게 값 전달

		DWORD dwRead1;
		TCHAR chBuf1[4096] = { 0, };

		ReadFile(YourRPipe, chBuf1, 4096, &dwRead1, NULL);
		printf(" he said : %s\n", chBuf1);

		readyqueue_put(i); // readyqueue에 프로세스 넣기

		memset(p[i]->logical.pagetable, 0, sizeof(p[i]->logical.pagetable));  // 빈 초기화 페이지 테이블 할당
		
		p[i]->p_number = i;
		p[i]->pid = pi.dwProcessId;
		p[i]->PWhandle = hWritePipe;
		p[i]->PRhandle = YourRPipe;
	}
	_tprintf(_T("[PARENT PROCESS]\n")); 
	
	


	for(int i = 0 ; i < 10 ; i ++)  // cpu burst, io burst, pid 전달 
	{
		char data1[4096] = { 0, };
		sprintf(data1, "%d", p[i]->cpu_burst);
		WriteFile(p[i]->PWhandle, data1, 4096, &bytesWritten, NULL);  // 자식 프로세스에게 cpu burst 전달
		char data2[4096] = { 0, };
		sprintf(data2, "%d", p[i]->io_burst);
		WriteFile(p[i]->PWhandle,data2, 4096, &bytesWritten, NULL);  // 자식 프로세스에게 io burst 전달
		char data3[4096] = { 0, };
		sprintf(data3, "%d", p[i]->pid);
		WriteFile(p[i]->PWhandle, data3, 4096, &bytesWritten, NULL);
		char data4[4096] = { 0, };
		sprintf(data4, "%d", p[i]->p_number);
		WriteFile(p[i]->PWhandle, data4, 4096, &bytesWritten, NULL);
	}


	


	// round robin //
	//scheduler 1차 텀프
	int time_quantum = 3;
	int count_time = 0;
	int time_save = 0;
	int i = 0;
	int round = 0;
	int count_demand = 0;

	int randomx = 0;
	while (1) {
		round++;
		fprintf(fp, "time quantum : %d \n\n", time_quantum);
		printf("---------------round %d----------------\n", round);
		fprintf(fp, "---------------round %d----------------\n", round);

		///////////////////////////////////////////여기서부터 running //////////////////////////////////////////
		i = readyqueue_get();  // ready queue에서 맨 처음 나오는 프로세스

		
		//자식으로부터 10페이지 요구 메시지 받기
		
		memset(chBuf2, 0, sizeof(chBuf2));
		if (p[i]->cpu_burst > 0) {
			ReadFile(p[i]->PRhandle, chBuf2, 4096, &dwRead1, NULL);
			printf("%s\n", chBuf2);
			fprintf(fp,"%s\n", chBuf2);

		}
				
		memset(chBuf2, 0, sizeof(chBuf2));
		ReadFile(p[i]->PRhandle, chBuf2, 4096, &dwRead1, NULL);
		int demand_from_child = atoi(chBuf2);
		printf("자식 프로세스 P%d로부터 %d 의 페이지 요구를 받았습니다.\n", i, demand_from_child);
		fprintf(fp, "자식 프로세스 P%d로부터 %d 의 페이지 요구를 받았습니다.\n", i, demand_from_child);

		//logical memory   
		randomx = rand() % 10;
		int num[10] = { 0, };
		int ttmp = 0;
		for (int k = 0; k < 11; k++) {
			if (k == randomx) {
				continue;
			}
			num[ttmp] = k;
			ttmp++;
		}

		for (int k = 0; k < demand_from_child; k++) {
			for (int o = 0; o < 4; o++) {
				p[i]->logical.logical_memory[k][o] = 10 * num[k];
				p[i]->logical.logical_memory[k][o] += o;
			}
				
		}
		
		
		/////////////여기서부터 메모리 할당/////////////////

		// 10 page 할당. 페이지 테이블 확인 후 
		//random = rand() % 80;  // 0~80 난수 생성
		int cnt = 0; // for page table
		int memcount = 0;  // 10페이지를 메모리에 올리기 위한 변수
		while (cnt < 11) {
			if (randomx == cnt) {  // 로지컬 메모리를 만들때 생성한 난수와 변수 memcount가 같으면 pagetable 증가
				cnt++;
				continue;
			}
			random = rand() % 80;  // 0~80 난수 생성


			if (p[i]->logical.pagetable[cnt][1] == 0) {  // valid bit 0 일때
				if (physical_memory[random][0] == 0) { // 피지컬 메모리 비어있으면
					for (int k = 0; k < 4; k++) {
						physical_memory[random][k] = p[i]->logical.logical_memory[memcount][k];  // 피지컬 메모리에 로지컬 메모리 올라감

					}
					p[i]->logical.pagetable[cnt][1] = 1;  // page table valid bit 1로 바꾸기
					p[i]->logical.pagetable[cnt][0] = random;  // page table 매핑되는 주소 저장
					physical_memory_process_and_LRU[random][0] = i;  // 어떤 프로세스가 피지컬 메모리 프레임에 올라왔는지 기록.
					physical_memory_process_and_LRU[random][1] = 1;

				}
				else { // 비어있지 않은 경우를 위함
					int check = 0;
					
					
					for (int k = 0; k < 80; k++) {
						if (physical_memory[k][0] == 0) { // 피지컬 메모리 비어있으면 
							for (int x = 0; x < 4; x++) {
								physical_memory[k][x] = p[i]->logical.logical_memory[memcount][x];  // 피지컬 메모리에 로지컬 메모리 올라감

							}
							p[i]->logical.pagetable[cnt][1] = 1;
							p[i]->logical.pagetable[cnt][0] = k;
							physical_memory_process_and_LRU[k][0] = i;  // 어떤 프로세스가 피지컬 메모리 프레임에 올라왔는지 기록.
							physical_memory_process_and_LRU[k][1] = 1;
							check = 1;
							break;
						}
					}
					if (check == 0) {// 비어 있지 않다면 PAGE REPLACEMENT
						int frame_number = 0; // 프레임 넘버
						int tmp = 0;  // 가장 오래전에 사용된 프레임 찾기위한 변수
						for (int k = 0; k < 80; k++) {
							if (tmp < physical_memory_process_and_LRU[k][1] && i!=physical_memory_process_and_LRU[k][0]) {  // 본인이 replace 되면 안되므로
								frame_number = k;
								tmp = physical_memory_process_and_LRU[k][1];
							}
						}

						printf("P%d의 프레임넘버 %d가 가장 오래돼서 replace 됨\n", physical_memory_process_and_LRU[frame_number][0], frame_number);
						fprintf(fp,"P%d의 프레임넘버 %d가 가장 오래돼서 replace 됨\n", physical_memory_process_and_LRU[frame_number][0], frame_number);

						int finding_index = 0;
						for (int k = 0; k < 11; k++) // page table의 valid 비트를 바꾸기 위함
						{
							if (p[physical_memory_process_and_LRU[frame_number][0]]->logical.pagetable[k][0] == frame_number) {
								finding_index = k;
								break;
							}
						}
						p[physical_memory_process_and_LRU[frame_number][0]]->logical.pagetable[finding_index][1] = 0; // valid bit 0으로 바꾸기


						p[i]->logical.pagetable[cnt][1] = 1;
						p[i]->logical.pagetable[cnt][0] = frame_number;
						physical_memory_process_and_LRU[frame_number][0] = i;  // 어떤 프로세스가 피지컬 메모리 프레임에 올라왔는지 기록.
						physical_memory_process_and_LRU[frame_number][1] = 1;


						for (int k = 0; k < 4; k++) {
							physical_memory[frame_number][k] = p[i]->logical.logical_memory[memcount][k];  // 피지컬 메모리에 로지컬 메모리 올라감

						}

					}


				}

			}

			memcount++;
			cnt++;
		}

		for (int k = 0; k < 80; k++) {  //오래된 프레임 구하기 위한 코드
			if (physical_memory_process_and_LRU[k][1] != 0) {
				physical_memory_process_and_LRU[k][1] += 1;
			}
		}




		/////////////여기까지 메모리 할당/////////////////


		

		if (p[i]->cpu_burst > time_quantum) { // time_quantum보다 cpu_burst가 더 크면
			p[i]->cpu_burst -= time_quantum;
			time_save += time_quantum;
			count_time = time_quantum;
		}
		else {
			time_save += p[i]->cpu_burst;
			count_time = p[i]->cpu_burst;
			p[i]->cpu_burst = 0;
		}
		printf("이번 라운드의 running queue에서 pid : %d인 P%d의 cpu burst %d 소모\n", p[i]->pid, i, count_time);
		printf("P%d의 남은 cpu burst는 %d \n", i, p[i]->cpu_burst);
		fprintf(fp, "이번 라운드에서 pid : %d인 P%d의 cpuburst %d 소모\n", p[i]->pid, i, count_time);
		fprintf(fp, "P%d의 남은 cpu burst는 %d \n", i, p[i]->cpu_burst);

		printf("ready queue : ");
		fprintf(fp,"ready queue : ");

		//fprintf(fp, "ready queue : ");

		for (int i = 0; i < 11; i++) {
			printf("%d ", readyqueue[i]);
			fprintf(fp, "%d ", readyqueue[i]);

		}
		printf("\n");
		fprintf(fp,"\n");


		printf("front index : %d \n", front);
		fprintf(fp, "front index : %d \n", front);

		printf("rear index : %d \n", rear);
		fprintf(fp, "rear index : %d \n", rear);
		////////////////////////////////////// running ////////////////////////////////////////

		// readyqueue
		if (p[i]->cpu_burst > 0) {
			readyqueue_put(i);
		}

		//자식 프로세스에게 넘겨주기
		char ss[30] = "";
		_itoa(p[i]->cpu_burst, ss, 10);
		WriteFile(p[i]->PWhandle, ss, 4096, &bytesWritten, NULL);

		printf(" %d times gone \n", time_save);
		fprintf(fp, " %d times gone \n", time_save);
		int count = 0;

		
		
		
		
		// VA 띄우기
		printf("----P%d의 VA ---- \n", i);
		printf("               Virtual Address       \n");
		fprintf(fp,"----P%d의 VA ---- \n", i);
		fprintf(fp,"               Virtual Address       \n");
		for (int k = 0; k < 10; k++) {
			printf("page %2d      |  ", k);
			fprintf(fp,"page %2d      |  ", k);

			for (int x = 0; x < 4; x++) {
				printf("%2d ", p[i]->logical.logical_memory[k][x]);
				fprintf(fp,"%2d ", p[i]->logical.logical_memory[k][x]);

			}
			printf("\n");
			fprintf(fp,"\n");

		}


		printf("\n");
		printf("-----현재 physical memory------ \n");
		fprintf(fp,"\n");
		fprintf(fp,"-----현재 physical memory------ \n");
		for (int k = 0; k < 80; k++) {
			printf("frame number : %3d       |  ", k);
			fprintf(fp,"frame number : %3d       |  ", k);
			for (int x = 0; x < 4; x++) {
				printf("%3d    ", physical_memory[k][x]);
				fprintf(fp,"%3d    ", physical_memory[k][x]);
			}

			if (physical_memory_process_and_LRU[k][0] >= 0) {
				printf("|   P%d이(가) 소유중  | ", physical_memory_process_and_LRU[k][0]);
				printf(" 현재 %2d 만큼 오래됨 ", physical_memory_process_and_LRU[k][1]);
				printf("\n");
				fprintf(fp,"|   P%d이(가) 소유중  | ", physical_memory_process_and_LRU[k][0]);
				fprintf(fp," 현재 %2d 만큼 오래됨 ", physical_memory_process_and_LRU[k][1]);
				fprintf(fp,"\n");
			}
			else {
				printf("|    x   \n");
				fprintf(fp,"|    x   \n");

			}
		}


		for (int j = 0; j < 10; j++) {
			if (p[j]->cpu_burst == 0) {
				count++;
			}
		}

		if (count == 10) break;

	}

	CloseHandle(pi.hProcess);
	CloseHandle(hReadPipe);
	CloseHandle(hWritePipe);

	for (int i = 0; i < sizeof(p) / sizeof(struct PCB *); i++)    // 요소 개수만큼 반복
	{
		free(p[i]);    // 각 요소의 동적 메모리 해제
	}

	Sleep(5000);
	return 0;
}
