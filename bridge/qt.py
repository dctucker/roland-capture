#!/usr/bin/env python3

from model import Model

import sys
import math
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *

model = Model(create=False)
labels = [
	'  Mic ',
	'Guitar',
	'Deluge',
	'Opsix',
	'Phatty',
	' SM10 ',
	'Circuit',
	' JUNO ',
]

class GLWidget(QOpenGLWidget):
	def __init__(self, parent=None):
		QOpenGLWidget.__init__(self, parent)

		self.painter = QPainter()
		self.pen_grey = QPen(QColor(192,192,192))
		self.pen_black = QPen(QColor(0,0,0))
		self.font = QFont('ChicagoFLF', 9)

		self.setFixedSize(800,400)
		self.glrect = QRect(0,0,800,400)
		self.background = QBrush(QColor(1,1,1))
		self.foreground = QBrush(QColor(200,200,0))
		self.pen_fg = QPen(self.foreground.color())
		self.stereo_on = QBrush(QColor(0,200,200))
		self.button_on  = QBrush(QColor(200,0,0))
		self.button_off = QBrush(QColor(50,50,50))
		self.pan_brush = QBrush(QColor(224,128,0))
		self.pan_pen = QPen(self.pan_brush.color())

		self.iteration = 0
		self.timer = QTimer(self)
		self.timer.timeout.connect(self.timeout)
		self.timer.start(64)

	def repaint(self):
		self.iteration += 1
		painter = self.painter
		painter.begin(self)
		painter.setFont(self.font)
		size = self.glrect
		painter.fillRect(QRect(0, 0, size.width(), size.height()), self.background)
		width = int(size.width())
		col_width = width // 16
		col_spacing = 5
		row_height = 30

		for ch in range(0, 16, 2):
			left = ch * col_width
			top = row_height

			painter.setPen(self.pen_grey)
			rect = QRect(left, top, col_width * 2, 20)
			painter.drawText(rect, Qt.AlignHCenter, labels[ch//2])
			top += row_height

			if model.stereo[ch+1] > 0:
				color = self.stereo_on
				rect = QRect(left+2*col_spacing, top, col_width * 2 - 2*col_spacing, 20)
				painter.fillRect(rect, color)
				painter.setPen(self.pen_black)
				painter.drawText(rect, Qt.AlignHCenter, "%d  /  %d" % (ch+1, ch+2))
			else:
				color = self.button_off
				rect = QRect(left+2*col_spacing, top, col_width - 2*col_spacing, 20)
				painter.fillRect(rect, color)
				painter.setPen(self.pen_grey)
				painter.drawText(rect, Qt.AlignHCenter, "%d" % (ch+1))

				left = (ch+1) * col_width
				rect = QRect(left+2*col_spacing, top, col_width - 2*col_spacing, 20)
				painter.fillRect(rect, color)
				painter.drawText(rect, Qt.AlignHCenter, "%d" % (ch+2))

		for ch in range(0, 16):
			left = ch * col_width
			center = col_spacing // 2 + left + col_width // 2
			top = 3 * row_height
			p = int((col_width - col_spacing) * model.pans[ch+1]['a'] / 200.0)
			painter.fillRect(QRect(left+col_spacing, top, col_width-col_spacing, 20), self.button_off)
			#painter.fillRect(QRect(center-1, top, 3, 20), self.pan_brush)
			pan_top = int(top+20-7 - abs(p / math.tan(math.radians(60))))
			poly = QPolygon((QPoint(center, top+20), QPoint(center+p, top+20), QPoint(center+p, pan_top)))
			painter.setBrush(self.pan_brush)
			painter.setPen(self.pan_pen)
			painter.drawPolygon(poly)
			#painter.fillRect(QRect(center, top, p, 20), self.pan_brush)

			for i, m in enumerate(model.monitors):
				top = 5 * row_height + i * row_height
				color = self.button_on if model.mutes[ch+1][m] > 0 else self.button_off
				if model.solos[ch+1][m]:
					color = QColor(self.stereo_on)
					fg = self.pen_black
				else:
					fg = self.pen_black if model.mutes[ch+1][m] > 0 else self.pen_fg
				painter.fillRect(QRect(left + col_spacing, top, col_width - col_spacing, 20), color)
				painter.setPen(fg)
				painter.drawText(center - col_spacing//2, top+15, m.upper())
		
		#painter.fillRect(QRect(0, 0, self.iteration % size.width(), 20), self.foreground)

		meter_top = top + row_height
		meter_height = size.height() - meter_top
		min_db = -60
		def bar_gradient(left):
			grad = QLinearGradient(
				QPoint(left, meter_top),
				QPoint(left - col_spacing * 2, meter_top + meter_height)
			)
			grad.setColorAt(0.125, QColor(255,0,0))
			grad.setColorAt(0.25 , QColor(255,255,0))
			grad.setColorAt(0.375, QColor(0,255,0))
			grad.setColorAt(0.5  , QColor(0,127,0))
			grad.setColorAt(0.75 , QColor(0,63,95))
			grad.setColorAt(1.0  , QColor(0,0,31))
			return grad

		for x in range(0, 16):
			rms = model.meters[x]
			#color = bar_color(rms)
			left = x * col_width + col_spacing
			top = meter_top + int(meter_height * rms / min_db)
			painter.fillRect(QRect(left + col_spacing, top, col_width - col_spacing * 2, meter_height), QBrush(bar_gradient(left)))
			painter.setPen(QPen(self.foreground.color()))
			painter.drawText(QRect(left, size.height() - 20, col_width, 20), Qt.AlignHCenter, "%d" % rms)
		#painter.fillRect(QRect(0, 0, self.iteration % size.width(), 20), self.foreground)

		painter.end()

	def paintEvent(self, event):
		self.glrect = event.rect()
		self.repaint()

	def timeout(self):
		self.update()

app = QApplication(sys.argv)
gl = GLWidget()
gl.show()

try:
	code = app.exec_()
except KeyboardInterrupt:
	code = 0
	pass

model.close()
sys.exit(code)
