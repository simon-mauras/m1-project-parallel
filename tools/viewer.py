#! /usr/bin/python3

import argparse
import matplotlib
import matplotlib.animation as anm
import matplotlib.pyplot as plt
import numpy
import sys
import time

current_milli_time = lambda: int(round(time.time() * 1000))

# # =======================================================

parser = argparse.ArgumentParser()
parser.add_argument('-i',		'--in',				dest='input',			type=str,																																						help='raw (binary) input file')
parser.add_argument('-o',		'--out',			dest='output',		type=str,																																						help='output path')
parser.add_argument('-s',		'--size',			dest='size',			type=int,		nargs=2,	default=[128,128],																						help='size')
parser.add_argument('-n',		'--numframe',	dest='numframe',	type=int,							default=0,																										help='number of frames')
parser.add_argument('-f',		'--format',		dest='format',		type=str,							default='png',	choices=['pdf', 'svg', 'png', 'mp4', 'show'],	help='output format: pdf, svg or png.')
parser.add_argument('-min',	'--min',			dest='min',				type=float,						default=-1.0,																									help='color map minimum value (optional)')
parser.add_argument('-max',	'--max',			dest='max',				type=float,						default=+1.0,																									help='color map maximum value (optional)')
args = parser.parse_args()


if not args.input:
	parser.error('Arguments -i is required')
if not args.output and args.format != 'show':
	parser.error('Arguments -o is required')

if args.format == 'svg':
	matplotlib.use('svg')
elif args.format == 'pdf':
	matplotlib.use('pdf')

###################################################################################################
#                              C L A S S E S   D E C L A R A T I O N                              #
###################################################################################################

class Data:
	def readRawFile(path, size):
		return numpy.fromfile(path).reshape(size)


def set_foregroundcolor(ax, color):
	for tl in ax.get_xticklines() + ax.get_yticklines():
		tl.set_color(color)
	for spine in ax.spines:
		ax.spines[spine].set_edgecolor(color)
	for tick in ax.xaxis.get_major_ticks():
		tick.label1.set_color(color)
	for tick in ax.yaxis.get_major_ticks():
		tick.label1.set_color(color)
	ax.axes.xaxis.label.set_color(color)
	ax.axes.yaxis.label.set_color(color)
	ax.axes.xaxis.get_offset_text().set_color(color)
	ax.axes.yaxis.get_offset_text().set_color(color)
	ax.axes.title.set_color(color)
	lh = ax.get_legend()
	if lh != None:
		lh.get_title().set_color(color)
		lh.legendPatch.set_edgecolor('none')
		labels = lh.get_texts()
		for lab in labels:
			lab.set_color(color)
	for tl in ax.get_xticklabels():
		tl.set_color(color)
	for tl in ax.get_yticklabels():
		tl.set_color(color)

def set_backgroundcolor(ax, color):
	ax.patch.set_facecolor(color)
	lh = ax.get_legend()
	if lh != None:
		lh.legendPatch.set_facecolor(color)

class Window:
	def __init__(self, figsize=None, backgroundcolor='black', foregroundcolor='white'):
		self.fig, self.axes = plt.subplots(nrows=1, ncols=1, figsize=figsize, facecolor=backgroundcolor)
		self.colors = { 'backgroundcolor': backgroundcolor, 'foregroundcolor': foregroundcolor }
	def addImg(self, args):
		self.objects = Img(self.axes, args)
		self.axes.set_title(args.title)
		self.axes.axis(args.axis)
		self.axes.set_xticks([])
		self.axes.set_yticks([])
		set_foregroundcolor(self.axes, self.colors['foregroundcolor'])
		set_backgroundcolor(self.axes, self.colors['backgroundcolor'])
	def addBar(self, position, ticklocation='right'):
		cax = self.fig.add_axes(position)
		plt.colorbar(self.objects.img, cax=cax, ticklocation=ticklocation)
		set_foregroundcolor(cax, self.colors['foregroundcolor'])
		set_backgroundcolor(cax, self.colors['backgroundcolor'])
	def update(self, data):
		self.objects.set(data)
		return self
	def show(self):
		plt.show()
		return self
	def save(self, path):
		t1 = current_milli_time()
		plt.savefig(path, bbox_inches='tight', facecolor=self.fig.get_facecolor())
		t2 = current_milli_time()
		sys.stdout.write('- save:     %d ms\n' % (t2 - t1))
		return self

class Img:
	def __init__(self, axe, param):
		if param.min <= 0.0 and param.log:
			param.min = 0.01
		if param.log:
			self.img = axe.imshow(numpy.ones((args.size[1], args.size[0])), interpolation = 'none', cmap = param.cmap, norm=matplotlib.colors.LogNorm(), vmin=param.min, vmax=param.max)
		else:
			self.img = axe.imshow(numpy.ones((args.size[1], args.size[0])), interpolation = 'none', cmap = param.cmap, vmin=param.min, vmax=param.max)
	def set(self, data):
		self.img.set_data(data)

class Params:
	def __init__(self):
		self.title	= ''
		self.axis		= 'on'
		self.cmap		= 'hot'
		self.min		= 1
		self.max		= 100000
		self.log		= True
	def setTitle(self, title):
		self.title = title
		return self
	def setAxis(self, axis):
		self.axis = axis
		return self
	def setCmap(self, cmap):
		self.cmap = cmap
		return self
	def setMin(self, min):
		self.min = min
		return self
	def setMax(self, max):
		self.max = max
		return self
	def setLog(self, log):
		self.log = log
		return self

###############################################################################
#              E N V I R O N M E N T   C O N F I G U R A T I O N              #
###############################################################################


d_map = matplotlib.cm.get_cmap('seismic')
d_map.set_bad((0,0,0))

ratio = numpy.sqrt(args.size[0] / args.size[1])

win = Window(figsize = (10*ratio,10/ratio))
win.addImg(Params().setCmap(d_map).setMin(args.min).setMax(args.max).setLog(False))
win.addBar([0.93, 0.1, 0.03, 0.8], ticklocation='right'	)





def updateWindow(frame = 0):
	sys.stdout.write('[ Rendering frame %d ]\n' % frame)
	t1 = current_milli_time()
	if args.numframe is 0:
		data = Data.readRawFile(args.input, (args.size[1], args.size[0]))
	else:
		data = Data.readRawFile(args.input % frame, (args.size[1], args.size[0]))
	t2 = current_milli_time()
	sys.stdout.write('- input:    %d ms\n' % (t2 - t1))
	win.update(data)
	t3 = current_milli_time()
	sys.stdout.write('- update:   %d ms\n' % (t3 - t2))

if args.format == 'mp4':
	anim = anm.FuncAnimation(win.fig, updateWindow, frames=args.numframe, interval=20, blit=True)
	anim.save(args.output, fps=30, extra_args=['-vcodec', 'h264', '-pix_fmt', 'yuv420p'], savefig_kwargs={'facecolor':win.fig.get_facecolor()})

elif args.format == 'show':
	updateWindow()
	win.show()

elif args.numframe is 0:
	t1 = current_milli_time()
	updateWindow()
	win.save(args.output)
	t2 = current_milli_time()
	sys.stdout.write('Total time: %d ms\n\n' % (t2 - t1))

else:
	for frame in range(0, args.numframe):
		t1 = current_milli_time()
		updateWindow(frame)
		win.save(args.output % frame)
		t2 = current_milli_time()
		sys.stdout.write('Total time: %d ms\n\n' % (t2 - t1))