#pragma once

template <class T>
class SingleInstance {
	friend T;
public:
	static T* GetInstance() {
		if (!instance) {
			instance = new T();
		}
		return instance;
	}
	~SingleInstance() {
		if (SingleInstance<T>::instance)
		{
			delete instance;
		}
	}
private:
	SingleInstance() = default;
	static T* instance;
};
template<class T>
T* SingleInstance<T>::instance = nullptr;