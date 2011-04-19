/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \brief Sample demonstration of URBI capabilities.
/*
 * This is a port to URBI of the Sony OPEN-R balltrackinghead example.
 * The algorithms used are as close as possible to the original version.
 */
#include <libport/cmath>

#include <vector>

#include <urbi/uclient.hh>
#include <urbi/uconversion.hh>

#ifndef LIBURBI_OPENR
# include "monitor.hh"
Monitor* mon = NULL;
#endif

//inline double fabs(double a) {return  a>0?a:(a*-1);}
inline double fsgn (double a)
{
  return a>0?1:-1;
}

struct PositionData
{
  int frame;
  double value;
  bool operator == (int f) {return f==frame;}
};


class BallTracking
{
 public:
  BallTracking(const char * robotName);

  //callback functions
  urbi::UCallbackAction getHead(bool pan, const urbi::UMessage &msg);
  urbi::UCallbackAction getImage(const urbi::UMessage &msg);

 private:
  /// Client for image reception.
  urbi::UClient robotI;
  /// Client for command sending.
  urbi::UClient robotC;
  /// Client for command reception.
  /// urbi::UClient robotG;

  //joint value for the last few frames
  std::vector<PositionData> current_x, current_y;
  // base of current_x, current_y (currentx[i] = frame_base - i)
  int baseframeX, baseframeY;
  unsigned char  image[500000];  //uncompressed image
  double target_x, target_y;  //command to center the ball
  double expect_x, expect_y;  //values in last command, that should be reached


  void doSendCommand(double current_x, double current_y);

  static const int VSIZE = 10; //size of the joint position list.
  static const int ball_treshold = 50;
  static const double factor_x; //x field of view
  static const double factor_y;  //y field of view
  static const double maxcommand_x;
  static const double maxcommand_y;
};

const double BallTracking::factor_x = 0.9 * 56.9;
const double BallTracking::factor_y = 0.9 * 45.2;
const double BallTracking::maxcommand_x = 150.0;
const double BallTracking::maxcommand_y = 150.0;

int format=1;

void BallTracking::doSendCommand(double current_x, double current_y)
{
  static int sframe=0;
  static unsigned int stime=0;
  double command_x=-1, command_y=-1;
  robotC.send("headPan.val = headPan.val + %lf"
              " & headTilt.val = headTilt.val + %lf,",
              target_x, target_y);
  if (! (sframe % 1000))
  {
    if (stime)
      robotC.printf("!! csps %f\n",
                    1000000.0/(float)(robotC.getCurrentTime()-stime));
    stime=robotC.getCurrentTime();
  }
  ++sframe;
  return;
  if (fabs(current_x-expect_x)< 100)
  {
    if (fabs(target_x - current_x) > maxcommand_x)
      command_x = current_x + maxcommand_x*fsgn(target_x - current_x);
    else
      command_x = target_x;
    if (fabs(command_x-current_x) > 0.0)
    {
      robotC.send("headPan.val = %lf,",command_x);
      expect_x = command_x;
    }
    else
      command_x = -1;
  }
  if (fabs(current_y-expect_y)< 100)
  {
    if (fabs(target_y - current_y) > maxcommand_y)
      command_y = current_y + maxcommand_y*fsgn(target_y - current_y);
    else
      command_y = target_y;
    if (fabs(command_y-current_y) > 0.0)
    {
      robotC.send("headTilt.val = %lf,",command_y);
      expect_y = command_y;
    }
    else
      command_y = -1;
  }

  if (command_x!=-1 || command_y!=-1)
  {
    if (! (sframe % 1000))
    {
      if (stime)
        robotC.printf("!! csps %f\n",
                      1000000.0/(float)(robotC.getCurrentTime()-stime));
      stime=robotC.getCurrentTime();
    }
    ++sframe;
  }
}


urbi::UCallbackAction
BallTracking::getHead(bool pan, const urbi::UMessage &msg)
{
  if (msg.type != urbi::MESSAGE_DATA
      || msg.value->type != urbi::DATA_DOUBLE)
    return urbi::URBI_CONTINUE;

  PositionData pd;
  pd.frame = msg.timestamp/32;
  pd.value = msg.value->val;
  if (pan)
  {
    current_x.insert(current_x.begin(), pd);
    current_x.resize(VSIZE);
  }
  else
  {
    current_y.insert(current_y.begin(), pd);
    current_y.resize(VSIZE);
  }

  return urbi::URBI_CONTINUE;
}

urbi::UCallbackAction
BallTracking::getImage(const urbi::UMessage &msg)
{
  static int framenum = 0;
  static float interframe = 0;
  static int frametime = 0;

  if (msg.type != urbi::MESSAGE_DATA
      || msg.value->type != urbi::DATA_BINARY
      || msg.value->binary->type != urbi::BINARY_IMAGE)
    return urbi::URBI_CONTINUE;

  urbi::UImage& img = msg.value->binary->image;
  double cx=0, cy=0;
  /*
    std::vector<PositionData>::iterator it =
    find(current_x.begin(), current_x.end(), msg.timestamp/32);
    if (it==current_x.end())
    {
    return URBI_CONTINUE;
    }
    double cx = it->value;
    it = find(current_y.begin(), current_y.end(), msg.timestamp/32);
    if (it==current_y.end())
    {
    return URBI_CONTINUE;
    }
    double cy = it->value;
  */
  if ((framenum % 50)==0)
  {
    if (!frametime)
      frametime = robotC.getCurrentTime();
    else
    {
      int dt = robotC.getCurrentTime() - frametime;
      frametime = robotC.getCurrentTime();
      if (interframe == 0)
        interframe = ((float)dt)/50.0;
      else
        interframe = interframe*0.5 +  0.5*((float)dt)/50.0;
      robotC.printf("## %f fps\n", 1000.0/interframe);
    }
  }
  ++framenum;
  size_t imgsize = 500000;
  if (img.imageFormat == urbi::IMAGE_JPEG)
  {
    size_t w, h;
#if defined __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wcast-align"
#endif
    urbi::convertJPEGtoYCrCb((const urbi::byte *) img.data, img.size,
                             (urbi::byte **) &image, imgsize
                             , w, h);
#if defined __clang__
# pragma clang diagnostic pop
#endif
  }
  else
    memcpy(image, img.data, img.width * img.height * 3);


  //get ball centroid
  int xsum=0, ysum=0;
  int nummatch=0;
  int w = img.width;
  int h = img.height;
  for (unsigned i=0;i<img.width; ++i)
    for (unsigned j=0;j<img.height; ++j)
    {
      unsigned char cb = image[(i+j*w)*3+1];
      unsigned char cr = image[(i+j*w)*3+2];;
      if (150 <= cr && cr<=230
          && 120 <= cb && cb<=190)
      {
        ++nummatch;
        xsum+=i;
        ysum+=j;
      }
    }
  if (nummatch >= ball_treshold)
  {
    double bx= (double)xsum / (double)nummatch;
    double by= (double)ysum / (double)nummatch;
    double dbx = bx - (double)w / 2.0;
    double dby = by - (double)h / 2.0;

    double dx = (-1.0) * (factor_x / (double)w) * dbx;
    double dy = (-1.0) * (factor_y / (double)h) * dby;

#ifndef LIBURBI_OPENR
    for (int j=0;j<h; ++j)
      image[(((int)bx)+w*j)*3]=255;
    for (int j=0;j<w; ++j)
      image[(((int)by)*w+j)*3]=255;
#endif

    target_x = cx+dx;
    target_y = cy+dy;
    if (target_x > 90.0)
      target_x = 90.0;
    if (target_x < -90.0)
      target_x = -90.0;
    if (target_y > 60.0)
      target_y = 60.0;
    if (target_y < -30.0)
      target_y = -30.0;
    doSendCommand(cx, cy);
  }

#ifndef LIBURBI_OPENR
  urbi::convertYCbCrtoRGB(image, w*h*3, image);
  if (!mon)
    mon = new Monitor(w, h, "Image");
  mon->setImage((bits8*)image, img.width*img.height*3);
#endif

  return urbi::URBI_CONTINUE;
}


BallTracking::BallTracking(const char * robotname)
  : robotI (robotname)
  , robotC (robotname)
    //, robotG (robotname)
{
  robotI.start();
  if (robotI.error())
    urbi::exit(1);
  robotC.start();
  if (robotC.error())
    urbi::exit(1);


  robotC.send("motoron;");
  robotC.send("camera.format = 1;");
#ifdef LIBURBI_OPENR
  robotC.send("camera.resolution = 1;");
#else
  robotC.send("camera.resolution = 0;");
#endif
  robotC.send("camera.jpegfactor = 75;");
  robotC.setCallback(*this, &BallTracking::getImage,"cam");

  robotC.send("loop cam << camera.val, ");
  //robotG.send("loop {pan << headPan.val& tilt << headTilt.val},");
}


int
main(int argc, char * argv[])
{
  const char* host = urbi::UClient::default_host();
  if (argc != 2)
    printf("usage: %s robotname\n", argv[0]);
  else
    host = argv[1];
  BallTracking bt(host);
  LIBPORT_USE(bt);
  urbi::execute();
}
