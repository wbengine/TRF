
#include "trf-sa-train.h"
using namespace trf;

char *cfg_pathVocab = NULL;
char *cfg_pathModelRead = NULL;
char *cfg_pathModelWrite = NULL;

int cfg_nThread = 1;

char *cfg_pathTest = NULL;

/* lmscore */
char *cfg_pathNbest = NULL;
char *cfg_writeLmscore = NULL;
char *cfg_writeLmscoreDebug = NULL;
char *cfg_writeTestID = NULL;

Option opt;
/* help */
const char *cfg_strHelp = "[Usage] : \n"
"Calculate log-likelihood:\n"
"  wbHRF -vocab [vocab] -read [model] -test [txt-id-file] -LL-method [Exact/Chib/AIS]\n"
"language model rescoring:\n"
"  wbHRF -vocab [vocab] -read [model] -test [nbest-list] -lmscore [output lmscore]\n"
"Normalizing: \n"
"  wbHRF -vocab [vocab] -read [model] -write [output model] -norm-method [Exact/AIS]\n"
"Reverse the length distribution pi:\n"
"  wbHRF -vocab [vcoab] -read [model] -write [output model] -pi-file [txt-id-file]\n"
"        count the length distribution in [txt-id-file] and set to the model\n"
;

#define  lout_exe lout<<"[TRF] "

double CalculateLL(Model &m, CorpusTxt *pCorpus, int nCorpusNum, double *pPPL = NULL);
void WordStr2ID(Array<VocabID> &aIDs, Array<String> &aStrs, LHash<const char*, VocabID> &vocabhash);
void LMRescore(Model &m, const char* pathTest);

_wbMain
{ 
	opt.m_strOtherHelp = cfg_strHelp;
	opt.Add(wbOPT_STRING, "vocab", &cfg_pathVocab, "The vocabulary");
	opt.Add(wbOPT_STRING, "read", &cfg_pathModelRead, "Read the init model to train");
	opt.Add(wbOPT_STRING, "write", &cfg_pathModelWrite, "output the normalizaed model");
	opt.Add(wbOPT_INT, "thread", &cfg_nThread, "The thread number");

	opt.Add(wbOPT_STRING, "test", &cfg_pathTest, "test corpus (TXT)");

	opt.Add(wbOPT_STRING, "nbest", &cfg_pathNbest, "nbest list (kaldi output)");
	opt.Add(wbOPT_STRING, "lmscore", &cfg_writeLmscore, "[LMrescore] output the lmsocre");
	opt.Add(wbOPT_STRING, "lmscore-debug", &cfg_writeLmscoreDebug, "[LMrescore] output the lmscore of each word for word-level combination");
	opt.Add(wbOPT_STRING, "lmscore-test-id", &cfg_writeTestID, "[LMrescore] output the vocab-id of test file");
	
	opt.Parse(_argc, _argv);

	lout << "*********************************************" << endl;
	lout << "              TRF.exe                        " << endl;
	lout << "\t" << __DATE__ << "\t" << __TIME__ << "\t" << endl;
	lout << "**********************************************" << endl;

	omp_set_num_threads(cfg_nThread);
	lout << "[OMP] omp_thread = " << omp_get_max_threads() << endl;
	srand(time(NULL));

	/// read model
	Vocab v(cfg_pathVocab);
	Model m(&v);
	lout_exe << "Read model: " << cfg_pathModelRead << endl;
	m.ReadT(cfg_pathModelRead);

	/* Operation 1: calculate LL */
	if (cfg_pathTest) {
		CorpusTxt *p = new CorpusTxt(cfg_pathTest);
		double dPPL;
		double dLL = CalculateLL(m, p, p->GetNum(), &dPPL);
		lout_exe << "calculate LL of : " << cfg_pathTest << endl;
		lout_exe << "-LL = " << -dLL << endl;
		lout_exe << "PPL = " << dPPL << endl;
		SAFE_DELETE(p);
	}

	/* Operation 2: lmscore */
	if (cfg_pathNbest) {
		LMRescore(m, cfg_pathNbest);
	}


	return 1;
}

double CalculateLL(Model &m, CorpusTxt *pCorpus, int nCorpusNum, double *pPPL /*= NULL*/)
{
	Array<double> aLL(omp_get_max_threads());
	aLL.Fill(0);

	Array<int> aWords(omp_get_max_threads());
	aWords.Fill(0);
	Array<int> aSents(omp_get_max_threads());
	aSents.Fill(0);

	Array<VocabID> aSeq;
	lout.Progress(0, true, nCorpusNum - 1, "omp GetLL");
#pragma omp parallel for firstprivate(aSeq)
	for (int i = 0; i < nCorpusNum; i++) {
		pCorpus->GetSeq(i, aSeq);

		Seq seq;
		seq.Set(aSeq, m.m_pVocab);
		LogP logprob = m.GetLogProb(seq);

		aLL[omp_get_thread_num()] += logprob;
		aWords[omp_get_thread_num()] += aSeq.GetNum();
		aSents[omp_get_thread_num()] += 1;

#pragma omp critical
		lout.Progress();
	}

	double dLL = aLL.Sum() / nCorpusNum;
	int nSent = aSents.Sum();
	int nWord = aWords.Sum();
	lout_variable(nSent);
	lout_variable(nWord);
	if (pPPL) *pPPL = exp(-dLL * nSent / (nSent + nWord));
	return dLL;
}

void WordStr2ID(Array<VocabID> &aIDs, Array<String> &aStrs, LHash<const char*, VocabID> &vocabhash)
{
	for (int i = 0; i < aStrs.GetNum(); i++) {
		String wstr = aStrs[i];
		VocabID *pvid = vocabhash.Find(wstr.Toupper());
		if (pvid == NULL) { // cannot find the word, then find <UNK>
			// as word has been saved into hash with uppor style, 
			// then we need to find <UNK>, not <unk>.
			pvid = vocabhash.Find("<UNK>");
			if (!pvid) {
				lout_error("Can't find a vocab-id of " << wstr.GetBuffer());
			}
		}
		aIDs[i] = *pvid;
	}
}

void LMRescore(Model &m, const char* pathTest)
{
	Vocab *pV = m.m_pVocab;

	/// hash the vocab
	LHash<const char*, VocabID> vocabhash;
	bool bFound;
	for (int i = 0; i < pV->GetSize(); i++) {
		int *pVID = vocabhash.Insert(String(pV->GetWordStr(i)).Toupper(), bFound);
		if (bFound) {
			lout_exe << "Find words with same name but different id! (str="
				<< pV->GetWordStr(i) << " id=" << i << ")" << endl;
			exit(1);
		}
	}

	/// rescore
	lout_exe << "Rescoring: " << pathTest << " ..." << endl;

	File fLmscore(cfg_writeLmscore, "wt");
	File fTestid(cfg_writeTestID, "wt");
	File file(pathTest, "rt");
	char *pLine;
	while (pLine = file.GetLine(true)) {
		String curLabel = strtok(pLine, " \t\n");
		String curSent = strtok(NULL, "\n");

		Array<String> aWordStrs;
		curSent.Split(aWordStrs, " \t\n");

		Array<VocabID> aWordIDs;
		WordStr2ID(aWordIDs, aWordStrs, vocabhash);

		Seq seq;
		seq.Set(aWordIDs, pV);
		LogP curLmscore = -m.GetLogProb(seq);

		/* output lmscore */
		fLmscore.Print("%s %lf\n", curLabel.GetBuffer(), curLmscore);
		/* output test-id */
		if (fTestid.Good()) {
			fTestid.Print("%s\t", curLabel.GetBuffer());
			fTestid.PrintArray("%d", aWordIDs.GetBuffer(), aWordIDs.GetNum());
		}
	}
}
