#include<iostream>

using namespace std;

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