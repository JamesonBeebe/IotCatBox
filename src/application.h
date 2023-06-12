#ifndef APPLICATION_H
#define APPLICATION_H

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