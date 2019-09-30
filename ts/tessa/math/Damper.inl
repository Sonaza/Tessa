
template <class T>
Damper<T>::Damper()
{
}

template <class T>
Damper<T>::Damper(const T &target, float smoothing)
	: m_currentValue(target)
	, m_targetValue(target)
	, m_smoothingFactor(smoothing)
{
}

// template <class T>
// Damper<T>::Damper(const Damper &other)
// {
// 	*this = other;
// }
// 
// template <class T>
// Damper &Damper<T>::operator=(const Damper &other)
// {
// 	if (this != &other)
// 	{
// 		m_currentValue = other.m_currentValue;
// 		m_targetValue = other.m_targetValue;
// 		m_smoothingFactor = other.m_smoothingFactor;
// 	}
// 	return *this;
// }

template <class T>
void Damper<T>::update(TimeSpan deltaTime)
{
// 	m_currentValue = lerp(m_currentValue, m_targetValue, 1.f - std::pow(m_smoothingFactor, deltaTime.getSecondsAsFloat()));
	m_currentValue = lerp(m_currentValue, m_targetValue, 1.f - std::exp(-m_smoothingFactor * deltaTime.getSecondsAsFloat()));
}

template <class T>
void Damper<T>::reset(const T &target, float smoothing)
{
	m_currentValue = target;
	m_targetValue = target;
	m_smoothingFactor = smoothing;
}

template <class T>
void Damper<T>::setSmoothingFactor(float smoothing)
{
	m_smoothingFactor = smoothing;
}

template <class T>
float Damper<T>::getSmoothingFactor() const
{
	return m_smoothingFactor;
}

template <class T>
void Damper<T>::setTarget(const T &target)
{
	m_targetValue = target;
}

template <class T>
const T &Damper<T>::getTarget() const
{
	return m_targetValue;
}

template <class T>
void Damper<T>::setValue(const T &value)
{
	m_currentValue = value;
}

template <class T>
const T &Damper<T>::getValue() const
{
	return m_currentValue;
}

template <class T>
void Damper<T>::cutToTarget()
{
	m_currentValue = m_targetValue;
}
