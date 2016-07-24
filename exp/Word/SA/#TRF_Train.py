import os
import math


dir = r'/home/wangbin/NAS/workspace/wangb/Experiments/Enwords_for_HRF/'
train = dir + r'LDC_gigaword_en.words.train.id'
valid = dir + r'LDC_gigaword_en.words.valid.id'
test = dir + r'LDC_gigaword_en.words.test.id'
vocab = dir + r'enword_c10.list'
order = 2
fs = 'g4_w_c_s_ls_tied.fs'
tmax = 1000
t0 = 500
minibatch = 300
gamma_lambda = "100,0.6"
gamma_zeta = "0,0.6"
thread = 2

if not os.path.exists('temp_model'):
	os.mkdir('temp_model')


for minibatch in [300]:
	cstr = ""
	vname = vocab[vocab.rfind('\\'):]
	cstr_beg = vname.rfind('_c')
	if cstr_beg != -1:
		cstr_end = vname.rfind('.')
		cstr = vname[cstr_beg: cstr_end]
		
	
	
	name = 'TRF_{}{}_SA'.format(fs[0:-3], cstr)
	print('***********',name, '*************')

	# input
	command = r'/home/wangbin/tools/trf_satrain '
	command += ' -vocab {} '.format(vocab)
	command += ' -train {} '.format(train)
	command += ' -valid {} '.format(valid)
	command += ' -test {} '.format(test)
	#command += ' -order {} '.format(order)
	command += ' -feat {} '.format(fs)

	# write
	command += ' -write {0}.model -log {0}.log '.format(name)
	# config
	command += ' -iter {} -t0 {} '.format(tmax, t0)
	command += ' -thread {} '.format(thread)	
	command += ' -mini-batch {} '.format(minibatch)
	command += ' -gamma-lambda {} -gamma-zeta {} '.format(gamma_lambda, gamma_zeta)
	command += ' -print-per-iter 1 '

	#print(command)

	os.system(command)
	
	