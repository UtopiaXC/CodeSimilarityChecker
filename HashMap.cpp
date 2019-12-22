/*
Code Similarity Checker
Github: https://github.com/UtopiaXC/CodeSimilarityChecker
IDE Visual Studio 2019 Community
UtopiaXC ©2019 All Rights Reserved
*/

#include<iostream>

using namespace std;

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