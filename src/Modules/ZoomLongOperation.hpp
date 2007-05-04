#ifndef __ZoomLongOperation_hpp__
#define __ZoomLongOperation_hpp__

class ZoomLongOperation : public LongOperation
{
private:
  const int m_fx; 
  const int m_fy; 
  const int m_fwidth; 
  const int m_fheight;
  const int m_tx; 
  const int m_ty; 
  const int m_twidth; 
  const int m_theight; 
  const int m_srcDC; 
  const int m_dx;
  const int m_dy; 
  const int m_dwidth; 
  const int m_dheight; 
  const int m_time;
    
public:
  ZoomLongOperation(
    const int fx, const int fy, const int fwidth, const int fheight, 
    const int tx, const int ty, const int twidth, const int theight,
    const int srcDC, const int dx, const int dy, const int dwidth, 
    const int dheight,  const int time);
  ~ZoomLongOperation();

  virtual bool operator()(RLMachine& machine);
};

#endif
