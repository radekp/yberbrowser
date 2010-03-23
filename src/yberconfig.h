#ifndef yberconfig_h
#define yberconfig_h

#define USE(FEAT) (defined USE_##FEAT  && USE_##FEAT)
#define ENABLE(FEAT) (defined ENABLE_##FEAT  && ENABLE_##FEAT)

#include <QDebug>
#endif
