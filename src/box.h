#ifndef BOX_H
#define BOX_H
#include <Arduino.h>

class Box {
  private:
    float boxWeight = 0; //Weight of the box without litter in Kg
    float sandWeight =0; //Amount of Litter in the box in Kg
    float currentWeight; //Box's current weight
    float lastWeight;   //last weight of the box
    byte uses=0;
    float pooWeight =0; //how much waste is in the box

  public:
    /*
    *   	Box Constructor - Default weight is 0.7 Kg
    */
    Box(float weight);
    ~Box();

    // Getter Functions
    float getCurrentWeight(void);
    float getLastWeight(void);
    float getBoxWeight(void);
    float getPooWeight(void);
    float getSandWeight(void);
    byte getUses(void);

    //Setter Functions
    void setCurrentWeight(float weight);
    void setLastWeight(float weight);
    void setPooWeight(float weight);
    float setSandWeight(float weight);    
    byte setUses(byte numUses);
    void incrementUses(void);

    //determine if there's a cat present
    bool catPresent(void);
};
#endif