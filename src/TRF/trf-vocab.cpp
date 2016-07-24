#include "trf-vocab.h"

namespace trf
{
	Vocab::Vocab()
	{
		m_aWords.Clean();
		// 		m_aWords[VocabID_seqbeg] = Word_beg;
		// 		m_aWords[VocabID_seqend] = Word_end;
	}
	Vocab::Vocab(const char* pathVocab)
	{
		int nNum = 0;
		int nClassNum = 0;

		File file(pathVocab, "rt");
		char *pLine;
		while (pLine = file.GetLine())
		{
			VocabID id = -1;
			char *pStr = NULL; // store the word string
			char *pClass = NULL; // store the class infor

			char *p = strtok(pLine, " \t\n");
			if (!p) {
				lout_warning("[Vocab] Empty Line! (nLine=" << file.nLine << ")");
				continue;
			}

			if (strcmp(p, Word_beg) == 0) {
				lout_error("[Vocab] the input vocab exists <s>! path=" << pathVocab);
			}
			else if (strcmp(p, Word_end) == 0) {
				lout_error("[Vocab] the input vocab exists </s>! path=" << pathVocab);
			}
			else {
				id = atoi(p);
				pStr = strtok(NULL, " \t\n");
				if (String(pStr, strlen("class=")) == "class=") {
					pClass = pStr;
					pStr = NULL;
				}
				else {
					pClass = strtok(NULL, " \t\n");
				}
			}

			if (id != nNum) {
				lout_error("[Vocab] The id is not continuous (id=" << id << ")(nNum=" << nNum << ")!");
			}
			m_aWords[id] = (pStr) ? pStr : "NAN";
			m_aWordID[id] = id;


			// get the class
			if (pClass) {
				pClass += strlen("class=");
				/* read the class information */
				m_aClass[id] = atoi(pClass);
				/* count the class number */
				nClassNum = max(nClassNum, m_aClass[id] + 1);
			}

			nNum++;
		}
		
		// get the class to words
		m_aClass2Word.SetNum(nClassNum);
		m_aClass2Word.Fill(NULL);
		for (int wid = 0; wid < m_aClass.GetNum(); wid++) {
			VocabID cid = m_aClass[wid];
			if (!m_aClass2Word[cid]) {
				m_aClass2Word[cid] = new Array<int>;
			}
			m_aClass2Word[cid]->Add(wid);
		}
		for (int cid = 0; cid < m_aClass2Word.GetNum(); cid++) {
			if (m_aClass2Word[cid] == NULL) {
				lout_error("[Vocab] class " << cid << " is empty!");
			}
		}


		lout << "[Vocab] Read from " << pathVocab << endl;
		lout << "[Vocab] Read " << nNum << " words" << endl;
		lout << "[Vocab] Class = " << m_aClass2Word.GetNum() << endl;
// 		for (int cid = 0; cid < m_aClass2Word.GetNum(); cid++) {
// 			lout << "[Vocab] cid=" << cid << "\t";
// 			lout.output(m_aClass2Word[cid]->GetBuffer(), m_aClass2Word[cid]->GetNum()) << endl;
// 		}
	}

	Vocab::~Vocab()
	{
		for (int i = 0; i < m_aClass2Word.GetNum(); i++) {
			SAFE_DELETE(m_aClass2Word[i]);
		}
		m_aClass2Word.Clean();
	}

	void Vocab::GetClass(VocabID *pcid, const VocabID *pwid, int nlen)
	{
		for (int i = 0; i < nlen; i++) {
			pcid[i] = GetClass(pwid[i]);
		}
	}
}