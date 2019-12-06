// 2015270209_����ȣ_childprocess
#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<stdlib.h>
#include<time.h>

#define BUFSIZE 1024


int pagetable[2][10] = { 0, };


int _tmain(INT argc, TCHAR *argv[]) {
	
	int cpu_burst = 0, io_burst = 0, pid = 0, process_number = 0;;

	//srand(time(NULL)); // ����
	

	HANDLE hRead = (HANDLE)atoi(argv[1]);
	_tprintf(_T("[Child Process]\n"));
	/* pipe�� �ٸ� ���� ���� �̿��� ������ ����*/
	PROCESS_INFORMATION pi;
	TCHAR recvString[4096] = { 0, };
	DWORD bytesRead;
	int a = 0;
	/*while (1) {
		if (a == 5 || !ReadFile(hRead, recvString, 4096, &bytesRead, NULL)) 
		{
			break;
		}
		_tprintf(_T("my cpu burst : %s \n"), recvString);
		Sleep(1000);
		a++;
	}
	*/
	ReadFile(hRead, recvString, 4096, &bytesRead, NULL);
	_tprintf(_T("received message : %s \n"), recvString);

	HANDLE WritePipe = (HANDLE)atoi(argv[2]);
	DWORD bytesWritten;
	char data[4096] = "yo";
	printf("my send message is %s\n", data);
	WriteFile(WritePipe, data, 4096, &bytesWritten, NULL);  // �ڽ� ���μ������� �� ����


	
	

	
	Sleep(500);
	ReadFile(hRead, recvString, 4096, &bytesRead, NULL);
	_tprintf(_T("received message : %s \n"), recvString);;


	char data1[4096] = "i'm child";
	printf("my send message is %s\n", data1);
	WriteFile(WritePipe, data1, 4096, &bytesWritten, NULL);
	

    // logical memory �䱸

	for (int i = 0; i < 10; i++) {

	}

	


	// cpu burst�� io burst ���� ���� 
	TCHAR recvString1[4096] = { 0, };
	TCHAR recvString2[4096] = { 0, };
	
	ReadFile(hRead, recvString1, 4096, &bytesRead, NULL);
	_tprintf(_T("my cpu burst : %s \n"), recvString1);
	cpu_burst = atoi(recvString1);

	ReadFile(hRead, recvString2, 4096, &bytesRead, NULL);
	_tprintf(_T("my io burst : %s \n"), recvString2);
	io_burst = atoi(recvString2);
	
	TCHAR PID[4096] = { 0, };
	ReadFile(hRead, PID, 4096, &bytesRead, NULL);
	pid = atoi(PID);
	_tprintf(_T("my pid : %s \n"), PID);

	TCHAR p_n[4096] = { 0, };
	ReadFile(hRead, p_n, 4096, &bytesRead, NULL);
	process_number = atoi(p_n);
	_tprintf(_T("I'm  P%s \n"), p_n);

	
	



	// cpu burst �ޱ� round robin
	TCHAR cpu_burst1[4096] = { 0, };
	/*
	ReadFile(hRead, cpu_burst1, 4096, &bytesRead, NULL);
	cpu_burst = atoi(cpu_burst1);
	_tprintf(_T("my cpu burst : %d \n"), cpu_burst);
	*/
	cpu_burst = 1;


	int count = 0; 

	while (cpu_burst > 0) {
		
		//ó���� Ÿ�� ƽ ���� �� 10 page �䱸
		
		TCHAR cmdString[4096] = { 0, };
		wsprintf(cmdString, _T("Hi! i'm P%d. I want 10 pages"), process_number);
		WriteFile(WritePipe, cmdString, 4096, &bytesWritten, NULL);


		char page10[4096] = { 0, };
		wsprintf(page10, _T("%d"), 10);
		WriteFile(WritePipe, page10, 4096, &bytesWritten, NULL);
		printf("i want %d pages\n", atoi(page10));
		
		
		
		
		memset(cpu_burst1, 0, sizeof(cpu_burst));
		ReadFile(hRead, cpu_burst1, 4096, &bytesRead, NULL);
		cpu_burst = atoi(cpu_burst1);
		_tprintf(_T("my cpu burst : %d \n"), cpu_burst);

		count++;
	}




	


	CloseHandle(hRead);
	system("pause");
	return 0;
	
}


