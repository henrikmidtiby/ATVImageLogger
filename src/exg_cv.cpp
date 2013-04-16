/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013  Morten Stigaard Laursen <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "exg_cv.h"
#include "exg_cv.moc"

ExG_cv::ExG_cv(void ): QThread()
{
  abort = false;
  this->start();
}

ExG_cv::~ExG_cv()
{
  abort = true;
  this->msleep(300);
}

void ExG_cv::run()
{
  int i =0;
  while(abort==false)
  {
    bool cont = false;
    while(cont == false)
    {
      cont = semImg.tryAcquire(1,100);
      if(abort == true)
	return;
    }
    cv::Mat img = Imgs.dequeue();
    cv::Mat exg;
//    QString fname("Img_" + QString::number(i++) + ".png");
//    cv::imwrite(fname.toLocal8Bit().constData(), BayerGR8);
    BayerGR16ToExG(img, exg);
    emit(newImage(exg));    
  }
}

void ExG_cv::BayerGR16ToExG(cv::InputArray in, cv::OutputArray out)
{
#pragma message BayerGR16ToExG only outputs raw bayer as exg now needs rewrite
  uint16_t * in_ = (uint16_t*)in.getMat().ptr();
  cv::Mat tmp_out(in.getMat().size().height, in.getMat().size().width, cv::DataType<uint8_t>::type);
  uint8_t * out_ = tmp_out.ptr();
  uint16_t * end = in_ + (in.getMat().size().height * in.getMat().size().width);
  while(in_!=end)
  {
    *out_++ = (*in_++)>>8;
  }
  tmp_out.copyTo(out);
}

void ExG_cv::newBayerGRImage(cv::Mat img)
{
  Imgs.enqueue(img);
  semImg.release(1);
}
