#pragma once

#include <initializer_list>
#include <vector>

template<class T>
struct KeyFrame {
    enum HandleType {
        SMOOTH, SMOOTH_IN, SMOOTH_OUT, STRAIGHT
    };

    KeyFrame(float time, T value, HandleType handleType = SMOOTH);

    float time;
    T value;
    HandleType handleType;
};

template<class T>
class Animation {
public:
    Animation(std::initializer_list<KeyFrame<T>> keyFrames);

    void update(float delta);
    void reset();

    T get() const;

private:
    std::vector<KeyFrame<T>> keyFrames;
    float time;
    unsigned int index;
};


template<class T>
KeyFrame<T>::KeyFrame(float time, T value, KeyFrame::HandleType handleType) :
    time(time), value(value), handleType(handleType)
{}

template<class T>
Animation<T>::Animation(std::initializer_list<KeyFrame<T> > keyFrames) :
    keyFrames(keyFrames), time(0), index(0)
{ }

template<class T>
void Animation<T>::update(float delta) {
    time += delta;
    while (index < keyFrames.size() && keyFrames[index].time < time) {
        index++;
    }
}

template<class T>
void Animation<T>::reset() {
    time = 0.0f;
    index = 0;
}

template<class T>
T Animation<T>::get() const {
    if (index >= keyFrames.size()) {
        return keyFrames.back().value;
    }

    if (index <= 0) {
        return keyFrames.front().value;
    }

    // TODO: smooth interpolation
    auto& a = keyFrames[index - 1], & b = keyFrames[index];
    float ratio = (time - a.time) / (b.time - a.time);
    return a.value * (1 - ratio) + b.value * ratio;
}
