#include <vector>
#include <set>

// util.h
typedef QVector<QColor> ColorMap;
typedef QPair<int,int> IntRange;
typedef QPair<qreal,qreal> RealRange;

// dataobject.h
struct indexedValue;

typedef unsigned long long ElemIndex;
typedef std::set<ElemIndex> ElemSet;
typedef std::vector<indexedValue> IndexList;

#ifndef ENUMS
#define ENUMS

// clustermetrics.h
enum METRIC_TYPE
{
    CORE_IMBALANCE = 0,
    L1_IMBALANCE,
    L2_IMBALANCE,
    L3_IMBALANCE,
    NUMA_IMBALANCE
};

#endif // ENUMS
