#include "lulesh.h"

#include <Mitos/Mitos.h>

#define SYM_BY_ELEM(v) \
    Mitos_add_symbol(#v, &v[0], sizeof(v[0]), elem_dims, 3)

#define SYM_BY_NODE(v) \
    Mitos_add_symbol(#v, &v[0], sizeof(v[0]), node_dims, 3)

void Domain::AddSymbolsToMitos() {

    size_t elem_dims[] = {m_sizeX, m_sizeY, m_sizeZ};
    size_t node_dims[] = {m_sizeX+1, m_sizeY+1, m_sizeZ+1};

    SYM_BY_NODE(m_x);
    SYM_BY_NODE(m_y);
    SYM_BY_NODE(m_z);

    SYM_BY_NODE(m_xd);
    SYM_BY_NODE(m_yd);
    SYM_BY_NODE(m_zd);

    SYM_BY_NODE(m_xdd);
    SYM_BY_NODE(m_ydd);
    SYM_BY_NODE(m_zdd);

    SYM_BY_NODE(m_fx);
    SYM_BY_NODE(m_fy);
    SYM_BY_NODE(m_fz);

    SYM_BY_NODE(m_nodalMass);

    SYM_BY_ELEM(m_lxim);
    SYM_BY_ELEM(m_lxip);
    SYM_BY_ELEM(m_letam);
    SYM_BY_ELEM(m_letap);
    SYM_BY_ELEM(m_lzetam);
    SYM_BY_ELEM(m_lzetap);

    SYM_BY_ELEM(m_elemBC);

    SYM_BY_ELEM(m_e);
    SYM_BY_ELEM(m_p);

    SYM_BY_ELEM(m_q);
    SYM_BY_ELEM(m_ql);
    SYM_BY_ELEM(m_qq);

    SYM_BY_ELEM(m_v);

    SYM_BY_ELEM(m_volo);
    SYM_BY_ELEM(m_delv);
    SYM_BY_ELEM(m_vdov);

    SYM_BY_ELEM(m_arealg);

    SYM_BY_ELEM(m_ss);

    SYM_BY_ELEM(m_elemMass);

    SYM_BY_ELEM(m_vnew);

    SYM_BY_ELEM(m_delx_xi);
    SYM_BY_ELEM(m_delx_eta);
    SYM_BY_ELEM(m_delx_zeta);

    SYM_BY_ELEM(m_delv_xi);
    SYM_BY_ELEM(m_delv_eta);
    SYM_BY_ELEM(m_delv_zeta);

    SYM_BY_ELEM(m_dxx);
    SYM_BY_ELEM(m_dyy);
    SYM_BY_ELEM(m_dzz);
}
