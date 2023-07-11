#ifndef BOX_H
#define BOX_H

#define SETTLING_TIME 7000

class Box {
  private:
    float boxWeight = 0; //Weight of the box without litter in Kg
    float sandWeight =0; //Amount of Litter in the box in Kg
    float currentWeight; //Box's current weight
    float lastWeight;   //last weight of the box
    float pooWeight = 0; //how much waste is in the box
    unsigned long settlingTime=0;
    bool isSettled;

  public:
    float lastStableWeight =0;
    
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

    //Setter Functions
    void setCurrentWeight(float weight);
    void setLastWeight(float weight);
    void setBoxWeight(float weight);
    void setPooWeight(float weight);
    float setSandWeight(float weight);    

    //determine if there's a cat present
    bool catPresent(void);
    void zeroPooValue(void);
    bool weightIsStable(void);
};
#endif