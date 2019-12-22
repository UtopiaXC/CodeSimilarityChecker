#include "CodeSimilarityChecker.h"

using namespace std;

//全局变量
const char ClassName[] = "CodeSimilarityChecker";//窗口类名
string cpp_keywords[48] = {
"if","int","for","do","new","try",
"asm","else","char","float","long","void",
"short","while","double","break","typedef","register",
"cintunue","catch","signed","unsigned","auto","static",
"extern","sizeof","delete","throw","const","class",
"friend","return","switch","public","union","goto",
"operator","template","enum","private","volatile","this",
"virtual","case","default","inline","protected","struct"
};//C++保留字

string ID_Key[100] = {
	"int","double","float","void","struct",
	"class" ,"char", "string", "short",
	"long" ,"auto","signed", "unsigned",
	"template","bool" };
int count_ID_Key = 15;

int cpp_keywords_count = 48;//检查的C++保留字总数
string id[100];//用户标识符
int id_count = 0;//标识符总数
HINSTANCE instance;//窗口全局句柄

//函数声明
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);//窗口回调函数

//散列表类，采用线性探测
class HashMap {
	//设定散列表最大值
#define HashMax 100
private:
	string keys[HashMax];
	int value[HashMax];
	int count;
	//散列函数
	int Hash(string keyword) {
		char key1 = keyword.at(0);
		char key2 = keyword.at(keyword.length() - 1);
		return (key1 * 25 + key2) % HashMax;
	}

public:
	//默认构造
	HashMap() {
		for (int i = 0; i < HashMax; i++) {
			keys[i] = "";
			value[i] = 0;
			count = 0;
		}
	}

	//重载构造，将关键字存入散列表
	HashMap(string keywords[], int count) {
		for (int i = 0; i < HashMax; i++) {
			keys[i] = "";
			value[i] = 0;
		}
		for (int i = 0; i < count; i++) {
			int key = Hash(keywords[i]);
			//线性探测
			while (keys[key] != "")
				key++;
			keys[key] = keywords[i];
			value[key] = 0;
		}
		this->count = count;
	}

	//将单词设置入散列表
	bool setHash(string keywords) {
		if (keywords == "" || count >= HashMax)
			return false;
		int key = Hash(keywords);
		//线性探测
		while (keys[key] != "")
			key++;
		keys[key] = keywords;
		value[key] = 0;
		return true;
	}

	//向类外传出散列函数
	int getHash(string keyword) {
		return Hash(keyword);
	}

	//获取散列表key对应的值
	int getValue(string keyword) {
		int key = Hash(keyword);
		//线性探测，如不存在返回-1，可用来判断是否入表
		while (keys[key] != keyword) {
			key++;
			if (key > HashMax)
				return -1;
		}
		return value[key];
	}

	//设置散列表key对应的值
	void setValue(string keyword, int value) {
		int key = Hash(keyword);
		//线性探测
		while (keys[key] != keyword) {
			key++;
			if (key > HashMax)
				return;
		}
		this->value[key] = value;
	}

	//获取散列表中保存项个数
	int getCount() {
		return this->count;
	}

};

//将将文件分解计入散列表
HashMap setHashMap(string location, HashMap * hashMap_id) {
	//将散列表初始化
	HashMap hashMap(cpp_keywords, cpp_keywords_count);
	//入文件流
	ifstream inStream;
	inStream.open(location);
	if (!inStream) {
		string error = "打开文件错误";
		throw error;
	}

	//标识符上文判断
	bool isIdentifier = false;
	bool isSpecialId = false;
	string identifier[100];
	int flag = 0;
	string code;
	//按行、空格读入
	while (!inStream.eof()) {
		inStream >> code;
		//将句尾添加空格来作为单单词句结束符
		code.append(" ");
		//保存找到的单词
		string word = "";
		for (char temp : code) {
			//判断是否为字母，如果是，将其插入单词末尾
			if ((temp > 96 && temp < 123) || (temp > 64 && temp < 91)) {
				word.append(1, temp);
			}
			//如果不是，则结束该单词
			else {
				//判断分隔符号是否为括号来排除函数名
				if (isIdentifier && (temp == '{' || temp == '(')&&!isSpecialId) {
					isIdentifier = false;
					flag = 0;
				}

				//判断是否存在标识符上文，存在则将标识符入表
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

				//判断是否为空单词
				if (word.length() != 0) {
					int value = hashMap.getValue(word);
					if (value != -1) {
						hashMap.setValue(word, value + 1);
						isKey = true;
					}
				}

				bool isNickKey = false;
				//如果单词为特殊保留字，则认为下文为标识符或函数名

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

				//如果是分割符号则认为下一单词可能也是标识符
				if (isIdentifier && (temp == ',' || temp == ' ')&&!isNickKey) {
					isNickKey = false;
					isIdentifier = true;
				}
			
			}



		}
	}
	return hashMap;
}

//将结果报告保存到文件
void solve(HashMap hashMap1, HashMap hashMap2, HashMap * id1, HashMap * id2, string location) {
	int edge1[100];
	int edge2[100];
	int sum1 = 0;
	int sum2 = 0;

	ofstream fout(location);

	fout << "保留字" << "," << "散列值" << "," << "文件1计数" << "," << "文件2计数" << "," << "差" << endl;
	for (int i = 0; i < cpp_keywords_count; i++) {
		int value1 = hashMap1.getValue(cpp_keywords[i]);
		int value2 = hashMap2.getValue(cpp_keywords[i]);
		edge1[i] = abs(value1 - value2);
		sum1 += edge1[i] * edge1[i];
		fout << cpp_keywords[i] << "," << hashMap1.getHash(cpp_keywords[i]) << "," << value1 << "," << value2 << "," << edge1[i] << endl;

	}

	fout << endl << "标识符" << "," << "散列值" << "," << "文件1计数" << "," << "文件2计数" << "," << "差" << endl;
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
	fout << endl << "保留字向量距离为：" << "," << sqrt(sum1) << endl;
	fout << "标识符向量距离为：" << "," << sqrt(sum2) << endl;
	double result = (sqrt(sum1) * 3 + sqrt(sum2)) / 2;
	fout << "综合平均距离为：" << "," << result << endl;
	fout << "综合平均距离权为：" << "," << "保留字向量/标识符向量=3/1" << endl << endl;

	//该判断抄袭算法不一定准确，但已通过部分实验
	if (result == 0.0)
		fout << "相似度为100%。" << endl << "两份代码完全一致。" << endl << "可认为完全抄袭。" << endl;
	else if (result >= 0 && result < 2.5)
		fout << "相似度为95%~100%。" << endl << "两份代码基本一致。" << endl << "可认为完全抄袭" << endl;
	else if (result >= 2.5 && result < 5)
		fout << "相似度为90%~95%。" << endl << "两份代码大部分一致。" << endl << "可认为存在抄袭" << endl;
	else if (result >= 5 && result < 10)
		fout << "相似度为80%~90%。" << endl << "两份代码存在一致。" << endl << "可能存在抄袭" << endl;
	else if (result >= 10 && result < 20)
		fout << "相似度为60%~80%。" << endl << "两份代码存在相似。" << endl << "可能存在借鉴" << endl;
	else if (result > 20)
		fout << "相似度小于60%。" << endl << "两份代码可认为不相似" << endl;
	fout << endl << "本报告结论有待考证。" << endl << "请结合表单判断。" << endl << "当代码量越大时该报告越准确。";
	fout.close();
}

//相似度对比功能函数
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

//程序入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	//注册窗体
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
		MessageBox(NULL, "窗体注册失败", "错误！", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	instance = hInstance;

	//创建窗体
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		ClassName,
		"C++代码相似度检查程序",
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 570, 420,
		NULL, NULL, hInstance, NULL);
	if (hwnd == NULL) {
		MessageBox(NULL, "窗体创建失败", "错误！", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	//主消息循环  
	while (GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

//回调
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		//关闭消息
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
		//销毁消息
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		//创建消息
	case WM_CREATE: {
		HWND hdlg = CreateDialog(instance, MAKEINTRESOURCE(IDD_MAINSURFACE), hwnd, (DLGPROC)WndProc);
		// 显示对话框
		ShowWindow(hdlg, SW_SHOWNA);
		break;
	}
					//命令消息
	case WM_COMMAND:
		switch (LOWORD(wParam)) {

			//关闭按钮触发
		case IDC_BUTTON_CLOSE:
			PostQuitMessage(0);
			break;

			//确认按钮触发
		case IDC_BUTTON_CONFIRM: {
			//通过EDIT获得文件地址
			TCHAR file1[MAX_PATH];
			GetDlgItemText(hwnd, IDC_EDIT1, file1, sizeof(file1));
			TCHAR file2[MAX_PATH];
			GetDlgItemText(hwnd, IDC_EDIT2, file2, sizeof(file2));

			//未选择文件触发
			if (file1[0] == _T('\0') || file2[0] == _T('\0')) {
				MessageBox(hwnd, "请先选择文件", "提醒！", NULL);
				break;
			}

			//保存报告位置
			TCHAR szPathName[MAX_PATH] = "代码相似性检查结果报告.csv";
			OPENFILENAME ofn = { OPENFILENAME_SIZE_VERSION_400 };//or  {sizeof (OPENFILENAME)}  
			ofn.hwndOwner = GetForegroundWindow();			// 拥有者句柄	
			ofn.lpstrFilter = TEXT("逗号分隔值文件(*.csv)");
			ofn.lpstrFile = szPathName;
			ofn.nMaxFile = sizeof(szPathName);
			ofn.lpstrTitle = TEXT("保存报告");
			ofn.Flags = OFN_OVERWRITEPROMPT;		// 覆盖提示
			BOOL bOk = GetSaveFileName(&ofn);
			if (!bOk) {
				return FALSE;
			}

			//检查是否成功保存
			if (!function(file1, file2, szPathName))
				MessageBox(hwnd, "源码文件打开失败！", "错误！", NULL);
			else {
				MessageBox(hwnd, "已导出报告，本报告结论有待考证，请结合表单判断。\n当代码量越大时该报告越准确。", "成功！", NULL);
			}
			break;
		}

								 //文件1浏览按钮触发
		case IDC_BUTTON1: {
			//文件选择结构体
			OPENFILENAME opfn;
			opfn.lpstrTitle = TEXT("请选择源码文件");
			WCHAR strFilename[MAX_PATH];//存放文件名
			//初始化
			ZeroMemory(&opfn, sizeof(OPENFILENAME));
			opfn.lStructSize = sizeof(OPENFILENAME);//结构体大小
			//设置过滤
			opfn.lpstrFilter = "C++源文件\0*.cpp\0文本文件\0*.txt\0C源文件\0*.c\0";
			//默认过滤器索引设为1
			opfn.nFilterIndex = 1;
			//文件名的字段必须先把第一个字符设为 \0
			opfn.lpstrFile = (LPSTR)strFilename;
			opfn.lpstrFile[0] = '\0';
			opfn.nMaxFile = sizeof(strFilename);
			//设置标志位，检查目录或文件是否存在
			opfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
			// 显示对话框让用户选择文件
			if (GetOpenFileName(&opfn)) {
				//在文本框中显示文件路径
				HWND hEdt = GetDlgItem(hwnd, IDC_EDIT1);
				SendMessage(hEdt, WM_SETTEXT, NULL, (LPARAM)strFilename);
			}
			break;
		}

						  //文件2浏览按钮触发
		case IDC_BUTTON2: {
			//文件选择结构体，同按钮1
			OPENFILENAME opfn;
			opfn.lpstrTitle = TEXT("请选择源码文件");
			WCHAR strFilename[MAX_PATH];
			ZeroMemory(&opfn, sizeof(OPENFILENAME));
			opfn.lStructSize = sizeof(OPENFILENAME);
			opfn.lpstrFilter = "C++源文件*.cpp\0*.cpp\0文本文件*txt\0*.txt\0C源文件*.c\0*.c\0";
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