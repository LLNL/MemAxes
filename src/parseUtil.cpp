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

int dseDepth(int enc)
{
    int src = enc & 0xF;
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

int dseDirty(int enc)
{
    int src = enc & 0xF;
    switch(src)
    {
        case(0x5): return 0; // from another core L2/L1 (clean)
        case(0x6): return 1; // from another core L2/L1 (dirty)
        case(0xA): return 0; // local RAM (clean)
        case(0xB): return 0; // remote RAM (clean)
        case(0xC): return 1; // local RAM (dirty)
        case(0xD): return 1; // remote RAM (dirty)
    }

    // N/A
    return -1;
}


std::string encToString(int enc)
{
    int src = enc & 0xF;

    switch(src)
    {
        case(0x0): return "Unknown L3 Miss";
        case(0x1): return "L1";
        case(0x2): return "TLB";
        case(0x3): return "L2";
        case(0x4): return "L3";
        case(0x5): return "L3 Snoop (clean)"; // from another core L2/L1
        case(0x6): return "L3 Snoop (dirty)"; // from another core L2/L1
        case(0x7): return "LLC Snoop (dirty)";
        case(0x8): return "L3 Miss (dirty)"; // local ram?
        case(0x9): return "MONKEYS"; // reserved (shouldn't happen)
        case(0xA): return "Local RAM";
        case(0xB): return "Remote RAM";
        case(0xC): return "Local RAM";
        case(0xD): return "Remote RAM";
        case(0xE): return "I/O";
        case(0xF): return "Uncacheable";
    }

    return "???"; // really weird
}


int dseSTLB(int enc)
{
    return enc & 0x10;
}

int dseLocked(int enc)
{
    return enc & 0x20;
}
