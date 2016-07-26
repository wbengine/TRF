#You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Copyright 2014-2015 Tsinghua University
# Author: wb.th08@gmail.com (Bin Wang), ozj@tsinghua.edu.cn (Zhijian Ou) 
#
# All h, cpp, cc, and script files (e.g. bat, sh, pl, py) should include the above 
# license declaration. Different coding language may use different comment styles.


import os
import math

#count the sentence number and the word number of a txt files
def file_count(fname):
	f = open(fname)
	nLine = 0
	nWord = 0
	for line in f:
		nLine += 1
		nWord += len(line.split())
	f.close()
	return [nLine, nWord]

# calculate the ppl to loglikelihood	
def PPL2LL(ppl, nLine, nWord):
	return -math.log(ppl) * (nLine+nWord) / nLine

dir = '../data/'
train = dir + 'training.id'
valid = dir + 'valid.id'
test = dir + 'test.id'
vocab = dir + 'ptb.lext'
cutoff = [0]*7
corpus = [train, valid, test]

# count txt
count = [0]*len(corpus)
for txt in corpus:
	count[corpus.index(txt)] = file_count(txt)

output = 'ngram.txt'
fout = open(output, 'wt')
fout.write('models   -LL-train   -LL-valid   -LL-test   PPL-tran   PPL-valid   PPL-test\n')

for order in [2,3,4,5]:
	lmname = '{}gram{}.lm'.format( order, ''.join([ str(i) for i in cutoff[0:order] ]) ) 
	print('>> ' + lmname);
	
	cmd = '../../../bin/ngram-count'
	cmd += ' -vocab {} '.format(vocab)
	cmd += ' -text {}'.format(train)
	cmd += ' -order {}'.format(order)
	cmd += ' -lm {} '.format(lmname)
	cmd += ' -wbdiscount -interpolate '
	for n in range(len(cutoff)):
		cmd += ' -gt{}min {} '.format(n+1, cutoff[n])
	
	os.system(cmd)
	
	ppl = [0]*len(corpus)
	LL = [0]*len(corpus)
	for txt in corpus:
		cmd = '../../../bin/ngram'
		cmd += ' -vocab {} '.format(vocab)
		cmd += ' -lm {} '.format(lmname)
		cmd += ' -order {}'.format(order)
		cmd += ' -ppl {} '.format(txt)
		res = os.popen(cmd).read()
		
		# find ppl
		toks = res.split()
		idx = toks.index('ppl=')
		if idx == -1:
			print('Output error!!!')
			print(res)
			exit()
		else:
			cur_ppl = float(toks[idx+1])
			
		# ppl to LL
		idx = corpus.index(txt)
		ppl[idx] = cur_ppl
		LL[idx] = PPL2LL(cur_ppl, count[idx][0], count[idx][1])
		
	# output the result LL and PPL
	outline = lmname[0:-3]
	for i in range(len(LL)):
		outline += '\t{:.2f}'.format(-LL[i])
	for i in range(len(ppl)):
		outline += '\t{:.2f}'.format(ppl[i])
		
	fout.write(outline+'\n')

	


