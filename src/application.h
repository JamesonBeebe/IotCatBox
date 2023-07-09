#ifndef APPLICATION_H
#define APPLICATION_H

#define MEASUREMENT_TOLERANCE 0.2       // Restart averaging if the weight changes more than 0.2 kg.
#define TRANSITION_THRESHOLD 2    // general threshold for switching states

#define SOFTWARE_VERSION_MAJOR  1
#define SOFTWARE_VERSION_MINOR  2

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