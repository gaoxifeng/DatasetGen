#ifndef CONSOLE_H
#define CONSOLE_H

#include "../Config.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <queue>
#include <map>

class FTFont;

PRJ_BEGIN

class DefaultConsole
{
public:
  DefaultConsole();
  void setColor(float r,float g,float b,float a,int sz);
  void addMsg(const std::string& msg,sizeType durOrId);
  void rmvMsg(sizeType id);
  virtual void reshape(int w,int h);
  virtual void render();
protected:
  void initDrawMsg();
  void finishDrawMsg();
  void drawMsg(const std::string& str);
  std::map<sizeType,std::string> _cmsgs;
  std::queue<std::pair<std::string,sizeType> > _msgs;
  int _w,_h,_sz;
  float _posx,_posy;
  float _r,_g,_b,_a;
};

PRJ_END

#endif
