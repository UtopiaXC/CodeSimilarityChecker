#include "CodeSimilarityChecker.h"

using namespace std;

//ȫ�ֱ���
const char ClassName[] = "CodeSimilarityChecker";//��������
string cpp_keywords[48] = {
"if","int","for","do","new","try",
"asm","else","char","float","long","void",
"short","while","double","break","typedef","register",
"cintunue","catch","signed","unsigned","auto","static",
"extern","sizeof","delete","throw","const","class",
"friend","return","switch","public","union","goto",
"operator","template","enum","private","volatile","this",
"virtual","case","default","inline","protected","struct"
};//C++������

string ID_Key[100] = {
	"int","double","float","void","struct",
	"class" ,"char", "string", "short",
	"long" ,"auto","signed", "unsigned",
	"template","bool" };
int count_ID_Key = 15;

int cpp_keywords_count = 48;//����C++����������
string id[100];//�û���ʶ��
int id_count = 0;//��ʶ������
HINSTANCE instance;//����ȫ�־��

//��������
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);//���ڻص�����

//ɢ�б��࣬��������̽��
class HashMap {
	//�趨ɢ�б����ֵ
#define HashMax 100
private:
	string keys[HashMax];
	int value[HashMax];
	int count;
	//ɢ�к���
	int Hash(string keyword) {
		char key1 = keyword.at(0);
		char key2 = keyword.at(keyword.length() - 1);
		return (key1 * 25 + key2) % HashMax;
	}

public:
	//Ĭ�Ϲ���
	HashMap() {
		for (int i = 0; i < HashMax; i++) {
			keys[i] = "";
			value[i] = 0;
			count = 0;
		}
	}

	//���ع��죬���ؼ��ִ���ɢ�б�
	HashMap(string keywords[], int count) {
		for (int i = 0; i < HashMax; i++) {
			keys[i] = "";
			value[i] = 0;
		}
		for (int i = 0; i < count; i++) {
			int key = Hash(keywords[i]);
			//����̽��
			while (keys[key] != "")
				key++;
			keys[key] = keywords[i];
			value[key] = 0;
		}
		this->count = count;
	}

	//������������ɢ�б�
	bool setHash(string keywords) {
		if (keywords == "" || count >= HashMax)
			return false;
		int key = Hash(keywords);
		//����̽��
		while (keys[key] != "")
			key++;
		keys[key] = keywords;
		value[key] = 0;
		return true;
	}

	//�����⴫��ɢ�к���
	int getHash(string keyword) {
		return Hash(keyword);
	}

	//��ȡɢ�б�key��Ӧ��ֵ
	int getValue(string keyword) {
		int key = Hash(keyword);
		//����̽�⣬�粻���ڷ���-1���������ж��Ƿ����
		while (keys[key] != keyword) {
			key++;
			if (key > HashMax)
				return -1;
		}
		return value[key];
	}

	//����ɢ�б�key��Ӧ��ֵ
	void setValue(string keyword, int value) {
		int key = Hash(keyword);
		//����̽��
		while (keys[key] != keyword) {
			key++;
			if (key > HashMax)
				return;
		}
		this->value[key] = value;
	}

	//��ȡɢ�б��б��������
	int getCount() {
		return this->count;
	}

};

//�����ļ��ֽ����ɢ�б�
HashMap setHashMap(string location, HashMap * hashMap_id) {
	//��ɢ�б��ʼ��
	HashMap hashMap(cpp_keywords, cpp_keywords_count);
	//���ļ���
	ifstream inStream;
	inStream.open(location);
	if (!inStream) {
		string error = "���ļ�����";
		throw error;
	}

	//��ʶ�������ж�
	bool isIdentifier = false;
	bool isSpecialId = false;
	string identifier[100];
	int flag = 0;
	string code;
	//���С��ո����
	while (!inStream.eof()) {
		inStream >> code;
		//����β��ӿո�����Ϊ�����ʾ������
		code.append(" ");
		//�����ҵ��ĵ���
		string word = "";
		for (char temp : code) {
			//�ж��Ƿ�Ϊ��ĸ������ǣ�������뵥��ĩβ
			if ((temp > 96 && temp < 123) || (temp > 64 && temp < 91)) {
				word.append(1, temp);
			}
			//������ǣ�������õ���
			else {
				//�жϷָ������Ƿ�Ϊ�������ų�������
				if (isIdentifier && (temp == '{' || temp == '(')&&!isSpecialId) {
					isIdentifier = false;
					flag = 0;
				}

				//�ж��Ƿ���ڱ�ʶ�����ģ������򽫱�ʶ�����
				if (isIdentifier && word.length() != 0) {
					hashMap_id->getHash(word);
					if (hashMap_id->getValue(word) != -1)
						hashMap_id->setValue(word, hashMap_id->getValue(word) + 1);
					else {
						hashMap_id->setHash(word);
						hashMap_id->setValue(word, 1);
						bool isInId = false;
						for (string temp : id) {
							if (temp == word)
								isInId = true;
						}
						if (!isInId)
							id[id_count++] = word;
					}
				}

				bool isKey = false;

				//�ж��Ƿ�Ϊ�յ���
				if (word.length() != 0) {
					int value = hashMap.getValue(word);
					if (value != -1) {
						hashMap.setValue(word, value + 1);
						isKey = true;
					}
				}

				bool isNickKey = false;
				//�������Ϊ���Ᵽ���֣�����Ϊ����Ϊ��ʶ��������

				for (string temp:ID_Key) 
					if (word == temp) {
						isIdentifier = true;
						isNickKey = true;
					}
					
				if (word == "struct" || word == "class")
						isSpecialId = true;
				
				
				else {
					isIdentifier = false;
					isSpecialId = false;
				}
				word = "";

				//����Ƿָ��������Ϊ��һ���ʿ���Ҳ�Ǳ�ʶ��
				if (isIdentifier && (temp == ',' || temp == ' ')&&!isNickKey) {
					isNickKey = false;
					isIdentifier = true;
				}
			
			}



		}
	}
	return hashMap;
}

//��������汣�浽�ļ�
void solve(HashMap hashMap1, HashMap hashMap2, HashMap * id1, HashMap * id2, string location) {
	int edge1[100];
	int edge2[100];
	int sum1 = 0;
	int sum2 = 0;

	ofstream fout(location);

	fout << "������" << "," << "ɢ��ֵ" << "," << "�ļ�1����" << "," << "�ļ�2����" << "," << "��" << endl;
	for (int i = 0; i < cpp_keywords_count; i++) {
		int value1 = hashMap1.getValue(cpp_keywords[i]);
		int value2 = hashMap2.getValue(cpp_keywords[i]);
		edge1[i] = abs(value1 - value2);
		sum1 += edge1[i] * edge1[i];
		fout << cpp_keywords[i] << "," << hashMap1.getHash(cpp_keywords[i]) << "," << value1 << "," << value2 << "," << edge1[i] << endl;

	}

	fout << endl << "��ʶ��" << "," << "ɢ��ֵ" << "," << "�ļ�1����" << "," << "�ļ�2����" << "," << "��" << endl;
	for (int i = 0; i < id_count; i++) {
		int id1_count = id1->getValue(id[i]);
		int id2_count = id2->getValue(id[i]);
		if (id1_count == -1)
			id1_count = 0;
		if (id2_count == -1)
			id2_count = 0;
		edge2[i] = abs(id1_count - id2_count);
		sum2 += edge2[i] * edge2[i];
		fout << id[i] << "," << id1->getHash(id[i]) << "," << id1_count << "," << id2_count << "," << edge2[i] << endl;

	}
	fout << endl << "��������������Ϊ��" << "," << sqrt(sum1) << endl;
	fout << "��ʶ����������Ϊ��" << "," << sqrt(sum2) << endl;
	double result = (sqrt(sum1) * 3 + sqrt(sum2)) / 2;
	fout << "�ۺ�ƽ������Ϊ��" << "," << result << endl;
	fout << "�ۺ�ƽ������ȨΪ��" << "," << "����������/��ʶ������=3/1" << endl << endl;

	//���жϳ�Ϯ�㷨��һ��׼ȷ������ͨ������ʵ��
	if (result == 0.0)
		fout << "���ƶ�Ϊ100%��" << endl << "���ݴ�����ȫһ�¡�" << endl << "����Ϊ��ȫ��Ϯ��" << endl;
	else if (result >= 0 && result < 2.5)
		fout << "���ƶ�Ϊ95%~100%��" << endl << "���ݴ������һ�¡�" << endl << "����Ϊ��ȫ��Ϯ" << endl;
	else if (result >= 2.5 && result < 5)
		fout << "���ƶ�Ϊ90%~95%��" << endl << "���ݴ���󲿷�һ�¡�" << endl << "����Ϊ���ڳ�Ϯ" << endl;
	else if (result >= 5 && result < 10)
		fout << "���ƶ�Ϊ80%~90%��" << endl << "���ݴ������һ�¡�" << endl << "���ܴ��ڳ�Ϯ" << endl;
	else if (result >= 10 && result < 20)
		fout << "���ƶ�Ϊ60%~80%��" << endl << "���ݴ���������ơ�" << endl << "���ܴ��ڽ��" << endl;
	else if (result > 20)
		fout << "���ƶ�С��60%��" << endl << "���ݴ������Ϊ������" << endl;
	fout << endl << "����������д���֤��" << endl << "���ϱ��жϡ�" << endl << "��������Խ��ʱ�ñ���Խ׼ȷ��";
	fout.close();
}

//���ƶȶԱȹ��ܺ���
bool function(TCHAR FILE1[], TCHAR FILE2[], TCHAR FILE_SAVE[]) {
	try {
		string location1 = FILE1;
		string location2 = FILE2;
		string save_location = FILE_SAVE;
		HashMap* hashMap_id1 = new HashMap();
		HashMap* hashMap_id2 = new HashMap();
		HashMap hashMap1 = setHashMap(location1, hashMap_id1);
		HashMap hashMap2 = setHashMap(location2, hashMap_id2);
		solve(hashMap1, hashMap2, hashMap_id1, hashMap_id2, save_location);
		return true;
	}
	catch (string error) {
		return false;
	}
}

//�������
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	//ע�ᴰ��
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, (LPCSTR)IDI_ICON);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(hInstance, (LPCSTR)IDI_ICON);
	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "����ע��ʧ��", "����", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	instance = hInstance;

	//��������
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		ClassName,
		"C++�������ƶȼ�����",
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 570, 420,
		NULL, NULL, hInstance, NULL);
	if (hwnd == NULL) {
		MessageBox(NULL, "���崴��ʧ��", "����", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	//����Ϣѭ��  
	while (GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

//�ص�
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		//�ر���Ϣ
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
		//������Ϣ
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		//������Ϣ
	case WM_CREATE: {
		HWND hdlg = CreateDialog(instance, MAKEINTRESOURCE(IDD_MAINSURFACE), hwnd, (DLGPROC)WndProc);
		// ��ʾ�Ի���
		ShowWindow(hdlg, SW_SHOWNA);
		break;
	}
					//������Ϣ
	case WM_COMMAND:
		switch (LOWORD(wParam)) {

			//�رհ�ť����
		case IDC_BUTTON_CLOSE:
			PostQuitMessage(0);
			break;

			//ȷ�ϰ�ť����
		case IDC_BUTTON_CONFIRM: {
			//ͨ��EDIT����ļ���ַ
			TCHAR file1[MAX_PATH];
			GetDlgItemText(hwnd, IDC_EDIT1, file1, sizeof(file1));
			TCHAR file2[MAX_PATH];
			GetDlgItemText(hwnd, IDC_EDIT2, file2, sizeof(file2));

			//δѡ���ļ�����
			if (file1[0] == _T('\0') || file2[0] == _T('\0')) {
				MessageBox(hwnd, "����ѡ���ļ�", "���ѣ�", NULL);
				break;
			}

			//���汨��λ��
			TCHAR szPathName[MAX_PATH] = "���������Լ��������.csv";
			OPENFILENAME ofn = { OPENFILENAME_SIZE_VERSION_400 };//or  {sizeof (OPENFILENAME)}  
			ofn.hwndOwner = GetForegroundWindow();			// ӵ���߾��	
			ofn.lpstrFilter = TEXT("���ŷָ�ֵ�ļ�(*.csv)");
			ofn.lpstrFile = szPathName;
			ofn.nMaxFile = sizeof(szPathName);
			ofn.lpstrTitle = TEXT("���汨��");
			ofn.Flags = OFN_OVERWRITEPROMPT;		// ������ʾ
			BOOL bOk = GetSaveFileName(&ofn);
			if (!bOk) {
				return FALSE;
			}

			//����Ƿ�ɹ�����
			if (!function(file1, file2, szPathName))
				MessageBox(hwnd, "Դ���ļ���ʧ�ܣ�", "����", NULL);
			else {
				MessageBox(hwnd, "�ѵ������棬����������д���֤�����ϱ��жϡ�\n��������Խ��ʱ�ñ���Խ׼ȷ��", "�ɹ���", NULL);
			}
			break;
		}

								 //�ļ�1�����ť����
		case IDC_BUTTON1: {
			//�ļ�ѡ��ṹ��
			OPENFILENAME opfn;
			opfn.lpstrTitle = TEXT("��ѡ��Դ���ļ�");
			WCHAR strFilename[MAX_PATH];//����ļ���
			//��ʼ��
			ZeroMemory(&opfn, sizeof(OPENFILENAME));
			opfn.lStructSize = sizeof(OPENFILENAME);//�ṹ���С
			//���ù���
			opfn.lpstrFilter = "C++Դ�ļ�\0*.cpp\0�ı��ļ�\0*.txt\0CԴ�ļ�\0*.c\0";
			//Ĭ�Ϲ�����������Ϊ1
			opfn.nFilterIndex = 1;
			//�ļ������ֶα����Ȱѵ�һ���ַ���Ϊ \0
			opfn.lpstrFile = (LPSTR)strFilename;
			opfn.lpstrFile[0] = '\0';
			opfn.nMaxFile = sizeof(strFilename);
			//���ñ�־λ�����Ŀ¼���ļ��Ƿ����
			opfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
			// ��ʾ�Ի������û�ѡ���ļ�
			if (GetOpenFileName(&opfn)) {
				//���ı�������ʾ�ļ�·��
				HWND hEdt = GetDlgItem(hwnd, IDC_EDIT1);
				SendMessage(hEdt, WM_SETTEXT, NULL, (LPARAM)strFilename);
			}
			break;
		}

						  //�ļ�2�����ť����
		case IDC_BUTTON2: {
			//�ļ�ѡ��ṹ�壬ͬ��ť1
			OPENFILENAME opfn;
			opfn.lpstrTitle = TEXT("��ѡ��Դ���ļ�");
			WCHAR strFilename[MAX_PATH];
			ZeroMemory(&opfn, sizeof(OPENFILENAME));
			opfn.lStructSize = sizeof(OPENFILENAME);
			opfn.lpstrFilter = "C++Դ�ļ�*.cpp\0*.cpp\0�ı��ļ�*txt\0*.txt\0CԴ�ļ�*.c\0*.c\0";
			opfn.nFilterIndex = 1;
			opfn.lpstrFile = (LPSTR)strFilename;
			opfn.lpstrFile[0] = '\0';
			opfn.nMaxFile = sizeof(strFilename);
			opfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
			if (GetOpenFileName(&opfn)) {
				HWND hEdt = GetDlgItem(hwnd, IDC_EDIT2);
				SendMessage(hEdt, WM_SETTEXT, NULL, (LPARAM)strFilename);
			}
			break;
		}
		}
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}