#pragma once
#include "wb-system.h"
using namespace wb;

namespace trf
{

	typedef double PValue;
	typedef double LogP;
	typedef double Prob;
	const float INF = 1e20;
	const float LogP_zero = -INF;

	/// ����Logp��probת��
	inline Prob LogP2Prob(LogP x) {
		return (x <= LogP_zero / 2) ? 0 : exp((double)(x));
	}
	inline LogP Prob2LogP(Prob x) {
		return ((x) <= 0) ? LogP_zero : log((double)(x));
	}
	///����log[exp(x) + exp(y)]
	inline LogP Log_Sum(LogP x, LogP y) {
		return (x > y) ? x + Prob2LogP(1 + LogP2Prob(y - x)) : y + Prob2LogP(1 + LogP2Prob(x - y));
	}
	/// ����log[exp(x)-exp(y)]
	inline LogP Log_Sub(LogP x, LogP y) {
		return (x > y) ? x + Prob2LogP(1 - LogP2Prob(y - x)) : y + Prob2LogP( LogP2Prob(x - y) - 1);
	}
	/// log summate all the values in array
	inline LogP Log_Sum(LogP *p, int num) {
		LogP sum = LogP_zero;
		for (int i = 0; i < num; i++) {
			sum = Log_Sum(sum, p[i]);
		}
		return sum;
	}
	/// һά�ֲ���һ��
	/**
	* \param [in] pdProbs �ֲ�
	* \param [in] nNum ��������� 
	*/
	LogP LogLineNormalize(LogP* pdProbs, int nNum);
	/// һά�ֲ���һ��
	/**
	* \param [in] pdProbs �����ֲ�
	* \param [in] nNum ��������� 
	*/
	int LogLineSampling(const LogP* pdProbs, int nNum);
	/// һά����
	/**
	* \param [in] pdProbs �������ķֲ�����Ҫ��һ��
	* \param [in] nNum ���������, sample from 0 to nNum-1
	* \return �����õ���������
	*/
	void LineNormalize(Prob* pdProbs, int nNum);
	/// һά����
	/**
	* \param [in] pdProbs �������Ķ����ֲ��ֲ�����Ҫ��һ��
	* \param [in] nNum ���������, sample from 0 to nNum-1
	* \return �����õ���������
	*/
	int LineSampling(const Prob* pdProbs, int nNum);
	/// MH�����Ľ���
	bool Acceptable(Prob prob);
	/// position��������һ�����������ȡ��n�����������滻�������ǰn��λ��
	/**
	* \param [in] a ��Ҫ����������
	* \param [in] len ���鳤��
	* \param [out] n ��Ҫ�����ĸ���
	*/
	void RandomPos(int *a, int len, int n);

	/// get a random float between dmin and dmax
	double Rand(double dmin, double dmax);

	/// smooth a distribution
	void EasySmooth(Prob *p, int num, Prob threshold = 1e-5);

	/// Get the AIS intermediate factor beta_t
	/* t -> current intermediate number (from 0 to T-1); T -> total intermadiate number. */
	double GetAISFactor(int t, int T);

	/// calculate the sigmoid function f(x) = 1/(1+exp(-x))
	double SigmFunc(double x);
}
