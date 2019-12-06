// 2015270209_����ȣ_ROUND_ROBIN_����
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<stdlib.h>
#include<time.h>


#define BUFSIZE 1024


int physical_memory[80][4] = { 0, }; // ��80������  �� �����Ӵ� 4byte�� ����. 
int physical_memory_process_and_LRU[80][2] = { 0, };  // ������ �޸𸮿� � ���μ����� �ְ�, ������� ���� �ð� ���.  [80][0]�� process��ȣ [80][1]�� �ð�



struct Logical {
	int pagetable[11][2]; // ������ ���̺�
	int logical_memory[11][4];// �� 11 page   �� �������� 4byte�� ����.
};


struct PCB {
	HANDLE PWhandle; //parent handle
	HANDLE PRhandle; //child handle
	int cpu_burst;
	int io_burst;
	int pid;
	int p_number; // process ��ȣ 
	struct Logical logical;
};




///////////// ready queue ���� ////////////
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

	srand(time(NULL)); // ����
	int random = 0;


	FILE *fp = fopen("memory_management_dump.txt", "w");

	
	//����ü ������ �迭 ���
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



	//�̸����� ������
	HANDLE hReadPipe = NULL, hWritePipe = NULL;
	TCHAR recvString[100];
	DWORD bytesWritten;
	DWORD bytesRead;

	STARTUPINFO si = { 0, };
	PROCESS_INFORMATION pi;
	//si.cb = sizeof(si);


	//// 2�� �� ���� ////
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

	for (int i = 0; i < 10; i++) {  // �ڽ� ���μ��� 10�� ����

		// 2�� ���� //
		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;
		//


		/* ���� pipe ���� */
		if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 4096)) {
			printf("create pipe error\n");
			return -1;
		}

		//// 2�� ���� /////

		if (!CreatePipe(&YourRPipe, &YourWPipe, &saAttr, 4096)) {
			printf("create pipe error\n");
			return -1;
		}

		SetHandleInformation(YourRPipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

		/* pipe�� �ٸ� ���� ���� �̿��� ������ �۽�*/
		TCHAR cmdString[4096] = { 0, };
		SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		wsprintf(cmdString, _T("%s %u %u"), _T("childprocess.exe"), hReadPipe, YourWPipe);


		CreateProcess(NULL, cmdString, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);


		//write �ϴ� �κ�//
		char data[4096] = "hi";
		//_itoa(p[i]->cpu_burst, data, 10);

		printf("string send: %s \n", data);
		WriteFile(hWritePipe, data, 4096, &bytesWritten, NULL);  // �ڽ� ���μ������� �� ����
		printf("handle : %d\n", hReadPipe);

		DWORD dwRead;
		TCHAR chBuf[4096] = { 0, };

		ReadFile(YourRPipe, chBuf, 4096, &dwRead, NULL);

		printf("read file : %s\n", chBuf);


		char data1[4096] = "who are you";
		WriteFile(hWritePipe, data1, 4096, &bytesWritten, NULL);  // �ڽ� ���μ������� �� ����

		DWORD dwRead1;
		TCHAR chBuf1[4096] = { 0, };

		ReadFile(YourRPipe, chBuf1, 4096, &dwRead1, NULL);
		printf(" he said : %s\n", chBuf1);

		readyqueue_put(i); // readyqueue�� ���μ��� �ֱ�

		memset(p[i]->logical.pagetable, 0, sizeof(p[i]->logical.pagetable));  // �� �ʱ�ȭ ������ ���̺� �Ҵ�
		
		p[i]->p_number = i;
		p[i]->pid = pi.dwProcessId;
		p[i]->PWhandle = hWritePipe;
		p[i]->PRhandle = YourRPipe;
	}
	_tprintf(_T("[PARENT PROCESS]\n")); 
	
	


	for(int i = 0 ; i < 10 ; i ++)  // cpu burst, io burst, pid ���� 
	{
		char data1[4096] = { 0, };
		sprintf(data1, "%d", p[i]->cpu_burst);
		WriteFile(p[i]->PWhandle, data1, 4096, &bytesWritten, NULL);  // �ڽ� ���μ������� cpu burst ����
		char data2[4096] = { 0, };
		sprintf(data2, "%d", p[i]->io_burst);
		WriteFile(p[i]->PWhandle,data2, 4096, &bytesWritten, NULL);  // �ڽ� ���μ������� io burst ����
		char data3[4096] = { 0, };
		sprintf(data3, "%d", p[i]->pid);
		WriteFile(p[i]->PWhandle, data3, 4096, &bytesWritten, NULL);
		char data4[4096] = { 0, };
		sprintf(data4, "%d", p[i]->p_number);
		WriteFile(p[i]->PWhandle, data4, 4096, &bytesWritten, NULL);
	}


	


	// round robin //
	//scheduler 1�� ����
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

		///////////////////////////////////////////���⼭���� running //////////////////////////////////////////
		i = readyqueue_get();  // ready queue���� �� ó�� ������ ���μ���

		
		//�ڽ����κ��� 10������ �䱸 �޽��� �ޱ�
		
		memset(chBuf2, 0, sizeof(chBuf2));
		if (p[i]->cpu_burst > 0) {
			ReadFile(p[i]->PRhandle, chBuf2, 4096, &dwRead1, NULL);
			printf("%s\n", chBuf2);
			fprintf(fp,"%s\n", chBuf2);

		}
				
		memset(chBuf2, 0, sizeof(chBuf2));
		ReadFile(p[i]->PRhandle, chBuf2, 4096, &dwRead1, NULL);
		int demand_from_child = atoi(chBuf2);
		printf("�ڽ� ���μ��� P%d�κ��� %d �� ������ �䱸�� �޾ҽ��ϴ�.\n", i, demand_from_child);
		fprintf(fp, "�ڽ� ���μ��� P%d�κ��� %d �� ������ �䱸�� �޾ҽ��ϴ�.\n", i, demand_from_child);

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
		
		
		/////////////���⼭���� �޸� �Ҵ�/////////////////

		// 10 page �Ҵ�. ������ ���̺� Ȯ�� �� 
		//random = rand() % 80;  // 0~80 ���� ����
		int cnt = 0; // for page table
		int memcount = 0;  // 10�������� �޸𸮿� �ø��� ���� ����
		while (cnt < 11) {
			if (randomx == cnt) {  // ������ �޸𸮸� ���鶧 ������ ������ ���� memcount�� ������ pagetable ����
				cnt++;
				continue;
			}
			random = rand() % 80;  // 0~80 ���� ����


			if (p[i]->logical.pagetable[cnt][1] == 0) {  // valid bit 0 �϶�
				if (physical_memory[random][0] == 0) { // ������ �޸� ���������
					for (int k = 0; k < 4; k++) {
						physical_memory[random][k] = p[i]->logical.logical_memory[memcount][k];  // ������ �޸𸮿� ������ �޸� �ö�

					}
					p[i]->logical.pagetable[cnt][1] = 1;  // page table valid bit 1�� �ٲٱ�
					p[i]->logical.pagetable[cnt][0] = random;  // page table ���εǴ� �ּ� ����
					physical_memory_process_and_LRU[random][0] = i;  // � ���μ����� ������ �޸� �����ӿ� �ö�Դ��� ���.
					physical_memory_process_and_LRU[random][1] = 1;

				}
				else { // ������� ���� ��츦 ����
					int check = 0;
					
					
					for (int k = 0; k < 80; k++) {
						if (physical_memory[k][0] == 0) { // ������ �޸� ��������� 
							for (int x = 0; x < 4; x++) {
								physical_memory[k][x] = p[i]->logical.logical_memory[memcount][x];  // ������ �޸𸮿� ������ �޸� �ö�

							}
							p[i]->logical.pagetable[cnt][1] = 1;
							p[i]->logical.pagetable[cnt][0] = k;
							physical_memory_process_and_LRU[k][0] = i;  // � ���μ����� ������ �޸� �����ӿ� �ö�Դ��� ���.
							physical_memory_process_and_LRU[k][1] = 1;
							check = 1;
							break;
						}
					}
					if (check == 0) {// ��� ���� �ʴٸ� PAGE REPLACEMENT
						int frame_number = 0; // ������ �ѹ�
						int tmp = 0;  // ���� �������� ���� ������ ã������ ����
						for (int k = 0; k < 80; k++) {
							if (tmp < physical_memory_process_and_LRU[k][1] && i!=physical_memory_process_and_LRU[k][0]) {  // ������ replace �Ǹ� �ȵǹǷ�
								frame_number = k;
								tmp = physical_memory_process_and_LRU[k][1];
							}
						}

						printf("P%d�� �����ӳѹ� %d�� ���� �����ż� replace ��\n", physical_memory_process_and_LRU[frame_number][0], frame_number);
						fprintf(fp,"P%d�� �����ӳѹ� %d�� ���� �����ż� replace ��\n", physical_memory_process_and_LRU[frame_number][0], frame_number);

						int finding_index = 0;
						for (int k = 0; k < 11; k++) // page table�� valid ��Ʈ�� �ٲٱ� ����
						{
							if (p[physical_memory_process_and_LRU[frame_number][0]]->logical.pagetable[k][0] == frame_number) {
								finding_index = k;
								break;
							}
						}
						p[physical_memory_process_and_LRU[frame_number][0]]->logical.pagetable[finding_index][1] = 0; // valid bit 0���� �ٲٱ�


						p[i]->logical.pagetable[cnt][1] = 1;
						p[i]->logical.pagetable[cnt][0] = frame_number;
						physical_memory_process_and_LRU[frame_number][0] = i;  // � ���μ����� ������ �޸� �����ӿ� �ö�Դ��� ���.
						physical_memory_process_and_LRU[frame_number][1] = 1;


						for (int k = 0; k < 4; k++) {
							physical_memory[frame_number][k] = p[i]->logical.logical_memory[memcount][k];  // ������ �޸𸮿� ������ �޸� �ö�

						}

					}


				}

			}

			memcount++;
			cnt++;
		}

		for (int k = 0; k < 80; k++) {  //������ ������ ���ϱ� ���� �ڵ�
			if (physical_memory_process_and_LRU[k][1] != 0) {
				physical_memory_process_and_LRU[k][1] += 1;
			}
		}




		/////////////������� �޸� �Ҵ�/////////////////


		

		if (p[i]->cpu_burst > time_quantum) { // time_quantum���� cpu_burst�� �� ũ��
			p[i]->cpu_burst -= time_quantum;
			time_save += time_quantum;
			count_time = time_quantum;
		}
		else {
			time_save += p[i]->cpu_burst;
			count_time = p[i]->cpu_burst;
			p[i]->cpu_burst = 0;
		}
		printf("�̹� ������ running queue���� pid : %d�� P%d�� cpu burst %d �Ҹ�\n", p[i]->pid, i, count_time);
		printf("P%d�� ���� cpu burst�� %d \n", i, p[i]->cpu_burst);
		fprintf(fp, "�̹� ���忡�� pid : %d�� P%d�� cpuburst %d �Ҹ�\n", p[i]->pid, i, count_time);
		fprintf(fp, "P%d�� ���� cpu burst�� %d \n", i, p[i]->cpu_burst);

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

		//�ڽ� ���μ������� �Ѱ��ֱ�
		char ss[30] = "";
		_itoa(p[i]->cpu_burst, ss, 10);
		WriteFile(p[i]->PWhandle, ss, 4096, &bytesWritten, NULL);

		printf(" %d times gone \n", time_save);
		fprintf(fp, " %d times gone \n", time_save);
		int count = 0;

		
		
		
		
		// VA ����
		printf("----P%d�� VA ---- \n", i);
		printf("               Virtual Address       \n");
		fprintf(fp,"----P%d�� VA ---- \n", i);
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
		printf("-----���� physical memory------ \n");
		fprintf(fp,"\n");
		fprintf(fp,"-----���� physical memory------ \n");
		for (int k = 0; k < 80; k++) {
			printf("frame number : %3d       |  ", k);
			fprintf(fp,"frame number : %3d       |  ", k);
			for (int x = 0; x < 4; x++) {
				printf("%3d    ", physical_memory[k][x]);
				fprintf(fp,"%3d    ", physical_memory[k][x]);
			}

			if (physical_memory_process_and_LRU[k][0] >= 0) {
				printf("|   P%d��(��) ������  | ", physical_memory_process_and_LRU[k][0]);
				printf(" ���� %2d ��ŭ ������ ", physical_memory_process_and_LRU[k][1]);
				printf("\n");
				fprintf(fp,"|   P%d��(��) ������  | ", physical_memory_process_and_LRU[k][0]);
				fprintf(fp," ���� %2d ��ŭ ������ ", physical_memory_process_and_LRU[k][1]);
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

	for (int i = 0; i < sizeof(p) / sizeof(struct PCB *); i++)    // ��� ������ŭ �ݺ�
	{
		free(p[i]);    // �� ����� ���� �޸� ����
	}

	Sleep(5000);
	return 0;
}
