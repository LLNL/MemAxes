//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. Written by Alfredo
// Gimenez (alfredo.gimenez@gmail.com). LLNL-CODE-663358. All rights
// reserved.
//
// This file is part of MemAxes. For details, see
// https://github.com/scalability-tools/MemAxes
//
// Please also read this link – Our Notice and GNU Lesser General Public
// License. This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License (as
// published by the Free Software Foundation) version 2.1 dated February
// 1999.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms and
// conditions of the GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// OUR NOTICE AND TERMS AND CONDITIONS OF THE GNU GENERAL PUBLIC LICENSE
// Our Preamble Notice
// A. This notice is required to be provided under our contract with the
// U.S. Department of Energy (DOE). This work was produced at the Lawrence
// Livermore National Laboratory under Contract No. DE-AC52-07NA27344 with
// the DOE.
// B. Neither the United States Government nor Lawrence Livermore National
// Security, LLC nor any of their employees, makes any warranty, express or
// implied, or assumes any liability or responsibility for the accuracy,
// completeness, or usefulness of any information, apparatus, product, or
// process disclosed, or represents that its use would not infringe
// privately-owned rights.
//////////////////////////////////////////////////////////////////////////////

#include "parseUtil.h"
#include "util.h"

size_t createUniqueID(QVector<QString> &existing, QString name)
{
    for(int i=0; i<existing.size(); i++)
    {
        if(existing[i] == name)
            return i;
    }
    existing.push_back(name);
    return existing.size()-1;
}

int stringToDepth(QString str)
{
    if(str == "L1" || str == "LFB")
        return 1;
    else if(str == "L2")
        return 2;
    else if(str == "L3")
        return 3;
    else if(str == "Local RAM")
        return 4;
    else if(str == "Remote RAM 1 Hop")
        return 5;
    else if(str == "Remote RAM 2 Hops")
        return 6;
    return -1;
}

int dseToDepth(uint64_t dse)
{
    dse >>= PERF_MEM_LVL_SHIFT; // shift for mem lvl

    if(dse & PERF_MEM_LVL_L1)
       return 1;
    if(dse & PERF_MEM_LVL_LFB)
       return 1;
    if(dse & PERF_MEM_LVL_L2)
       return 2;
    if(dse & PERF_MEM_LVL_L3)
       return 3;
    if(dse & PERF_MEM_LVL_LOC_RAM)
       return 4;
    if(dse & PERF_MEM_LVL_REM_RAM1)
       return 5;
    if(dse & PERF_MEM_LVL_REM_RAM2)
       return 6;
    if(dse & PERF_MEM_LVL_REM_CCE1)
       return 5;
    if(dse & PERF_MEM_LVL_REM_CCE2)
       return 6;
    if(dse & PERF_MEM_LVL_NA)
        return -1;
    if(dse & PERF_MEM_LVL_IO)
       return -2;
    if(dse & PERF_MEM_LVL_UNC)
        return -3;

    return -4;
}

int PEBS_dseToDepth(uint64_t dse)
{
    int src = dse & 0xF;
    switch(src)
    {
        case(0x0): return -1; // at least L3
        case(0x1): return 1; // L1
        case(0x2): return 1; // cache hit pending (don't draw)
        case(0x3): return 2; // L2
        case(0x4): return 3; // L3
        case(0x5): return 3; // from another core L2/L1 (clean)
        case(0x6): return 3; // from another core L2/L1 (dirty)
        case(0x7): return -1; // no LLC now
        case(0x8): return 4; // local ram?
        case(0x9): return -1; // reserved (shouldn't happen)
        case(0xA): return 4; // local RAM (clean)
        case(0xB): return 4; // remote RAM (clean)
        case(0xC): return 4; // local RAM (dirty)
        case(0xD): return 4; // remote RAM (dirty)
        case(0xE): return -1; // I/O
        case(0xF): return -1; // Uncacheable
    }

    return -1;
}

