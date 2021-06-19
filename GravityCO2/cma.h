#ifndef _CMA_H_
#define _CMA_H_

class CMA
{
  public:
    CMA(int N);
    float addData(float data);
    float avg(void);
  protected:
    int maxFreq;
    int freq;
    float sum;
};

#endif
