#ifndef APPLICATION_H
#define APPLICATION_H

#define THRESHOLD 0.2       // Restart averaging if the weight changes more than 0.2 kg.

class Application {
  private:
    float platformWeight;

  public:
    Application(float weight);    //application with default of 1.8Kgs
    ~Application();

    float setPlatformWeight(float weight);
    float getPlatformWeight(void);
};
#endif