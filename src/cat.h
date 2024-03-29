#ifndef CAT_H
#define CAT_H
#include <Arduino.h>

enum Gender{
  FEMALE,
  MALE
};

class Cat {
  private:
    double minWeight; //Cat's expected minimum weight to never go under
    double maxWeight; //Cat's expected maximum weight to never go over
    double currentWeight; //Cat's current weight
    byte uses=0;
    Gender gender;

  public:
    // Constructor
    Cat(double minWeight, double maxWeight, Gender catGender);

    // Getter functions
    float getMinWeight(void);

    float getMaxWeight(void);

    float getCurrentWeight(void);

    byte getUses(void);

    byte setUses(byte numUses);
    void setCurrentWeight(float weight);
    void incrementUses(void);
};
#endif