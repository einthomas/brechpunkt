#pragma once

#include <initializer_list>
#include <vector>

enum class HandleType {
    SMOOTH, SMOOTH_IN, SMOOTH_OUT, STOP
};

enum class ActionType {
    STRETCH, LOOP
};

template<class T>
struct KeyFrame {
    KeyFrame(float time, T value, HandleType handleType = HandleType::SMOOTH);

    float time;
    T value;
    T tangent;
    HandleType handleType;
};

template<class T>
struct Action {
    std::vector<KeyFrame<T>> keyFrames;

    float duration;
    ActionType type;
};

template<class T>
struct Command {
    float time;
    const Action<T>& action;
};

template<class T>
class Animation {
public:
    Animation(std::initializer_list<KeyFrame<T>> keyFrames, float duration = 0);
    Animation(std::initializer_list<Command<T>> commands, float duration = 0);

    void update(float delta);
    void reset();

    T get() const;

private:
    void calculateTangents();

    std::vector<KeyFrame<T>> keyFrames;
    T value;
    float time;
    unsigned int index;

    float duration;
};


template<class T>
KeyFrame<T>::KeyFrame(float time, T value, HandleType handleType) :
    time(time), value(value), tangent(), handleType(handleType)
{}

template<class T>
Animation<T>::Animation(
    std::initializer_list<KeyFrame<T> > keyFrames, float duration
) :
    keyFrames(keyFrames), time(0), index(0), duration(duration)
{
    calculateTangents();
    this->duration = std::max(duration, this->keyFrames.back().time);
}

template<class T>
Animation<T>::Animation(std::initializer_list<Command<T> > commands, float duration
) :
    keyFrames(), time(0), index(0), duration(duration)
{
    for (auto& command : commands) {
        for (auto keyFrame : command.action.keyFrames) {
            keyFrame.time = command.time + keyFrame.time;
            keyFrames.push_back(keyFrame);
        }
        // TODO: looping, stretching
    }

    calculateTangents();
    this->duration = std::max(duration, this->keyFrames.back().time);
}

template<class T>
void Animation<T>::update(float delta) {
    time += delta;

    while (time >= duration) {
        time -= duration;
        index = 0;
    }

    while (index < keyFrames.size() && keyFrames[index].time < time) {
        index++;
    }

    if (index >= keyFrames.size()) {
        value = keyFrames.back().value;

    } else if (index == 0) {
        value = keyFrames.front().value;

    } else {
        auto& a = keyFrames[index - 1], & b = keyFrames[index];
        float duration = b.time - a.time;

        // https://en.wikipedia.org/wiki/Cubic_Hermite_spline
        float t = (time - a.time) / duration;
        float t2 = t * t;
        float t3 = t2 * t;

        value =
            a.value * (2 * t3 - 3 * t2 + 1) +
            a.tangent * ((t3 - 2 * t2 + t) * duration) +
            b.value * (-2 * t3 + 3 * t2) +
            b.tangent * ((t3 - t2) * duration);
    }
}

template<class T>
void Animation<T>::reset() {
    time = 0.0f;
    index = 0;
}

template<class T>
T Animation<T>::get() const {
    return value;
}

template<class T>
void Animation<T>::calculateTangents() {
    if (this->keyFrames.front().handleType == HandleType::SMOOTH) {
        // can't do SMOOTH_IN on first key frame
        this->keyFrames.front().handleType = HandleType::SMOOTH_OUT;
    }
    if (this->keyFrames.back().handleType == HandleType::SMOOTH) {
        // can't do SMOOTH_OUT on last key frame
        this->keyFrames.back().handleType = HandleType::SMOOTH_IN;
    }

    for (unsigned int i = 0; i < keyFrames.size(); i++) {
        auto& k = this->keyFrames[i];

        const auto* before = &k;
        const auto* after = &k;

        if (k.handleType == HandleType::SMOOTH) {
            before = &this->keyFrames[i - 1];
            after = &this->keyFrames[i + 1];

        } else if (k.handleType == HandleType::SMOOTH_OUT) {
            after = &this->keyFrames[i + 1];

        } else if (k.handleType == HandleType::SMOOTH_IN) {
            before = &this->keyFrames[i - 1];

        }

        if (k.handleType != HandleType::STOP) {
            float inverseTimeDerivative = 1.0f / (after->time - before->time);
            k.tangent =
                after->value * inverseTimeDerivative +
                before->value * -inverseTimeDerivative;
        }
    }
}
