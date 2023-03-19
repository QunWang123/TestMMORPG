#include "UserData.h"
#include "Misc/Paths.h"
#include "Containers/StringConv.h"
#include "MyGameInstance.h"
#include <io.h>

#define MAX_FILENAME_LEN  250

FVector __LeftPos;
int32 __EditorState = 0;//��ǰ״̬
S_MAP_BIN_BASE   __MapBin;//��ͼ����
std::map<int32, AMyActor*> __MyActor;;//���е�actor

extern void insertBinValue(int row, int col, int id)
{
	if (row >= MAX_ROW || row < 0) return;
	if (col >= MAX_COL || col < 0) return;

	__MapBin.value[row][col] = id;
}
#pragma optimize("", off)
extern void deleteBinValue(int row, int col)
{
	if (row >= MAX_ROW || row < 0) return;
	if (col >= MAX_COL || col < 0) return;
	__MapBin.value[row][col] = 0;
}
#pragma optimize("", on)

extern void deleteBinValue(int id)
{
	for (int i = 0; i < MAX_ROW; i++)
	{
		for (int j = 0; j < MAX_COL; j++)
		{
			if (__MapBin.value[i][j] == id)
			{
				UBlueprintFunction::editor_DeleteActor(i, j);
			}
		}
	}
}

//д2��������
void writeBinData(char* filename)
{
	FString basedir = FPaths::ProjectDir();
	basedir = FPaths::Combine(*basedir, TEXT("Binaries/Win64/bin/"));

	TCHAR* pSendData = basedir.GetCharArray().GetData();
	int nDataLen = FCString::Strlen(pSendData);
	char* binPath = (char*)TCHAR_TO_ANSI(pSendData);

	if (__MapBin.row >= MAX_ROW || __MapBin.row <= 0) return;
	if (__MapBin.col >= MAX_COL || __MapBin.col <= 0) return;

	//*****************************************************
	//��ֵ
	int writeBytes = __MapBin.row * __MapBin.col;
	char* data = new char[writeBytes];
	memset(data, 0, writeBytes);

	for (int i = 0; i < __MapBin.row; i++)
	{
		for (int j = 0; j < __MapBin.col; j++)
		{
			uint8  v = __MapBin.value[i][j];
			int index = i * __MapBin.row + j;
			data[index] = v;
		}
	}


	//��ȡ·��
	char fname[MAX_FILENAME_LEN];
	memset(fname, 0, MAX_FILENAME_LEN);
	sprintf_s(fname, "%s//%s", binPath, filename);

	//�ж��ļ�������
	int isexist = _access(binPath, 0);
	if (isexist == -1)
	{
		delete[] data;
		return;
	}
	isexist = _access(fname, 0);
	//1 ���ļ�
	FILE * m_File = NULL;
	if (isexist == -1)
	{
		m_File = _fsopen(fname, "wb+", _SH_DENYNO);
		if (m_File == NULL) return;
		fseek(m_File, 0, SEEK_SET);
		bool  iswrite = true;//�Ƿ����д��
		fwrite(&iswrite, 1, sizeof(bool), m_File);
		fclose(m_File);
	}

	//2 д20���ֽڵ��ļ���
	m_File = _fsopen(fname, "rb+", _SH_DENYNO);
	fseek(m_File, 0, SEEK_SET);
	size_t ret_size = fwrite(&__MapBin, 1, 20, m_File);

	//3 ָ��ƫ�Ƶ�����20��λ�ÿ�ʼд�ļ�
	fseek(m_File, 20, SEEK_SET);
	fwrite(data, 1, writeBytes, m_File);
	fclose(m_File);

	//���ɾ��
	delete[] data;
}


void readBinData(char* filename)
{
	FString basedir = FPaths::ProjectDir();
	basedir = FPaths::Combine(*basedir, TEXT("Binaries/Win64/bin/"));
	TCHAR* pSendData = basedir.GetCharArray().GetData();
	int nDataLen = FCString::Strlen(pSendData);
	char* binPath = (char*)TCHAR_TO_ANSI(pSendData);


	__MapBin.reset();
	//��ȡ·��
	char fname[MAX_FILENAME_LEN];
	memset(fname, 0, MAX_FILENAME_LEN);
	sprintf_s(fname, "%s//%s", binPath, filename);
	int isexist = _access(binPath, 0);
	if (isexist == -1) return;
	isexist = _access(fname, 0);
	//1 ���ļ�
	if (isexist == -1) return;


	//1 �ȶ�20���ֽ����ݵ�mapbin
	FILE * m_File = _fsopen(fname, "rb+", _SH_DENYNO);
	fseek(m_File, 0, SEEK_SET);
	fread(&__MapBin, 1, 20, m_File);

	if (__MapBin.row >= MAX_ROW || __MapBin.row <= 0) return;
	if (__MapBin.col >= MAX_COL || __MapBin.col <= 0) return;

	//*****************************************************************
	int readBytes = __MapBin.row * __MapBin.col;
	char* data = new char[readBytes];

	fseek(m_File, 20, SEEK_SET);
	fread(data, 1, readBytes, m_File);

	//int pos = ftell(m_File);
	//fseek(m_File, 0, SEEK_END);
	//int len = ftell(m_File);

	fclose(m_File);

	//��ֵ������ 60��60��
	for (int i = 0; i < readBytes; i++)
	{
		int row = i / __MapBin.row;
		int col = i % __MapBin.row;
		__MapBin.value[row][col] = data[i];

	}


	//FString ss = FString::Printf(TEXT("testRead %d/%d %d/%d/%d  row:%d/%d value:%d/%d"), len, readBytes,
	//	__MapBin.left_pos.x, __MapBin.left_pos.y, __MapBin.left_pos.z, __MapBin.row, __MapBin.col,
	//	__MapBin.value[1][1], __MapBin.value[29][29]);
	//if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 10.0f, FColor::Emerald, ss);

	//֪ͨ��ͼ���ݼ������ 
	__AppGameInstance->editor_LoadDataOver();

	//���ɾ��
	delete[] data;
}