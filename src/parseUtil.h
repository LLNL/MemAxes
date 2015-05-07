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

#include <QVector>
#include <QString>

#include <stdint.h>

/* perf_event.h */
/* memory hierarchy (memory level, hit or miss) */
#define PERF_MEM_LVL_NA     0x01  /* not available */
#define PERF_MEM_LVL_HIT    0x02  /* hit level */
#define PERF_MEM_LVL_MISS   0x04  /* miss level  */
#define PERF_MEM_LVL_L1     0x08  /* L1 */
#define PERF_MEM_LVL_LFB    0x10  /* Line Fill Buffer */
#define PERF_MEM_LVL_L2     0x20  /* L2 */
#define PERF_MEM_LVL_L3     0x40  /* L3 */
#define PERF_MEM_LVL_LOC_RAM    0x80  /* Local DRAM */
#define PERF_MEM_LVL_REM_RAM1   0x100 /* Remote DRAM (1 hop) */
#define PERF_MEM_LVL_REM_RAM2   0x200 /* Remote DRAM (2 hops) */
#define PERF_MEM_LVL_REM_CCE1   0x400 /* Remote Cache (1 hop) */
#define PERF_MEM_LVL_REM_CCE2   0x800 /* Remote Cache (2 hops) */
#define PERF_MEM_LVL_IO     0x1000 /* I/O memory */
#define PERF_MEM_LVL_UNC    0x2000 /* Uncached memory */
#define PERF_MEM_LVL_SHIFT  5

size_t createUniqueID(QVector<QString> &existing, QString name);
int stringToDepth(QString str);
int dseToDepth(uint64_t dse);
int PEBS_dseToDepth(uint64_t dse);
