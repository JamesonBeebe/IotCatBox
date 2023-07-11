#include "application.h"

Application::Application(float weight){
    this->platformWeight = weight;
}

Application::~Application(){
}

float Application::setPlatformWeight(float weight){
    this->platformWeight = weight;
    return this->platformWeight;
}


float Application::getPlatformWeight(void){
    return this->platformWeight;
}