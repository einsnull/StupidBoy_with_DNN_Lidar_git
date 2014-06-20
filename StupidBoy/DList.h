#pragma once
template<class T>
class lNode
{
public:
	T elem;
	lNode *next,*prev;
};

template<class T>
class DList
{
private:
	int maxCnt;
	int cnt;
	lNode<T>* pHead;
	lNode<T>* pCur;
	lNode<T>* pPrev;
public:
	int getCnt()
	{
		return cnt;
	}
	int getMaxCnt()
	{
		return maxCnt;
	}
	DList(int maxCnt)
	{
		this->maxCnt = maxCnt;
		this->cnt = 0;
		pHead = new lNode<T>;//�½���ͷ
		pHead ->next = NULL;
		pHead ->prev = NULL;
		pCur = pHead;
	}
	~DList()
	{
		delete pHead;
	}
	void addTail(T e)
	{
		if(cnt >= maxCnt)
		{
			delHead();//delHead ʹcnt - 1 Ϊά��cnt���䣬��ʱ��
			//cnt��Ҫ�� 1 ���ԣ���ʹcnt == maxCnt��ʱ�� cntҲҪ+1
		}
		pCur->next = new lNode<T>;
		pCur->next->prev = pCur;
		pCur = pCur -> next;
		pCur->next = NULL;
		pCur->elem = e;
		cnt++; // cnt + 1
	}
	void delHead()//ɾ����Ԫ
	{
		if(cnt > 0)
		{
			lNode<T>* pTmp = pHead->next;
			pHead->next = pTmp -> next;
			if(pHead->next != NULL)
			{
				pHead->next->prev = pHead;
			}
			//cout << "del val:" << pTmp->elem <<endl;
			delete pTmp;
			cnt --;
		}
	}
	bool empty()
	{
		if(cnt > 0)
		{
			return false;
		}
		return true;
	}
	void print()
	{
		lNode<T>* p = pHead->next;
		while(p != NULL)
		{
			cout << p->elem << endl;
			p = p->next;
		}
	}
	bool tailValue(T &e)
	{
		if(pCur != NULL)
		{
			e = pCur->elem;
			return true;
		}
		return false;
	}
	void delTail()
	{
		if(pCur != pHead)
		{
			lNode<T>* pTmp = pCur->prev;
			pTmp->next = NULL;
			delete pCur;
			pCur = pTmp;
			cnt --;
		}
	}
};