/*-------------------------------------------------------------------------
  Author (C) 2008, Charles Wang <charlesw123456@gmail.com>

  This program is free software; you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the 
  Free Software Foundation; either version 2, or (at your option) any 
  later version.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
  for more details.

  You should have received a copy of the GNU General Public License along 
  with this program; if not, write to the Free Software Foundation, Inc., 
  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  As an exception, it is allowed to write an extension of Coco/R that is
  used as a plugin in non-free software.

  If not otherwise stated, any source code generated by Coco/R (other than 
  Coco/R itself) does not fall under the GNU General Public License.
-------------------------------------------------------------------------*/
#include  "syntax/Nodes.h"
#include  "Symbols.h"

static void
CcNodeT_Destruct(CcObject_t * self)
{
    CcNodeT_t * ccself = (CcNodeT_t *)self;
    if (ccself->pos) CcsPosition_Destruct(ccself->pos);
    CcNode_Destruct(self);
}
static const CcNodeType_t NodeT = {
    { sizeof(CcNodeT_t), "node_t", CcNodeT_Destruct }, CcNode_NoDeletable
};
const CcObjectType_t * node_t = (const CcObjectType_t *)&NodeT;
CcNode_t *
CcNodeT(int line, const CcSymbol_t * sym)
{
    CcNodeT_t * self = (CcNodeT_t *)CcNode(&NodeT, line);
    self->sym = sym;
    return (CcNode_t *)self;
}

static const CcNodeType_t NodePR = {
    { sizeof(CcNodePR_t), "node_pr", CcNode_Destruct }, CcNode_NoDeletable
};
const CcObjectType_t * node_pr = (const CcObjectType_t *)&NodePR;

static void
CcNodeNT_Destruct(CcObject_t * self)
{
    CcNodeNT_t * ccself = (CcNodeNT_t *)self;
    if (ccself->pos) CcsPosition_Destruct(ccself->pos);
    CcNode_Destruct(self);
}
static CcsBool_t
CcNodeNT_Deletable(CcNode_t * self)
{
    CcNodeNT_t * ccself = (CcNodeNT_t *)self;
    CcSymbolNT_t * sym = (CcSymbolNT_t *)ccself->sym;
    return sym->deletable;
}
static const CcNodeType_t NodeNT = {
    { sizeof(CcNodeNT_t), "node_nt", CcNodeNT_Destruct }, CcNodeNT_Deletable
};
const CcObjectType_t * node_nt = (const CcObjectType_t *)&NodeNT;
CcNode_t *
CcNodeNT(int line, const CcSymbol_t * sym)
{
    CcNodeNT_t * self = (CcNodeNT_t *)CcNode(&NodeNT, line);
    self->sym = sym;
    return (CcNode_t *)self;
}

static void
CcNodeWT_Destruct(CcObject_t * self)
{
    CcNodeWT_t * ccself = (CcNodeWT_t *)self;
    if (ccself->pos) CcsPosition_Destruct(ccself->pos);
    CcNode_Destruct(self);
}
static const CcNodeType_t NodeWT = {
    { sizeof(CcNodeWT_t), "node_wt", CcNodeWT_Destruct }, CcNode_NoDeletable
};
const CcObjectType_t * node_wt = (const CcObjectType_t *)&NodeWT;
CcNode_t *
CcNodeWT(int line, const CcSymbol_t * sym)
{
    CcNodeWT_t * self = (CcNodeWT_t *)CcNode(&NodeWT, line);
    self->sym = sym;
    return (CcNode_t *)self;
}

static void
CcNodeANY_Destruct(CcObject_t * self)
{
    CcNodeANY_t * ccself = (CcNodeANY_t *)self;
    if (ccself->set) CcBitArray_Destruct(ccself->set);
    CcNode_Destruct(self);
}
static const CcNodeType_t NodeANY = {
    { sizeof(CcNodeANY_t), "node_any", CcNodeANY_Destruct }, CcNode_NoDeletable
};
const CcObjectType_t * node_any = (const CcObjectType_t *)&NodeANY;

static void
CcNodeSYNC_Destruct(CcObject_t * self)
{
    CcNodeSYNC_t * ccself = (CcNodeSYNC_t *)self;
    if (ccself->set) CcBitArray_Destruct(ccself->set);
    CcNode_Destruct(self);
}
static const CcNodeType_t NodeSYNC = {
    { sizeof(CcNodeSYNC_t), "node_sync", CcNodeSYNC_Destruct },
    CcNode_Deletable
};
const CcObjectType_t * node_sync = (const CcObjectType_t *)&NodeSYNC;

static void
CcNodeSEM_Destruct(CcObject_t * self)
{
    CcNodeSEM_t * ccself = (CcNodeSEM_t *)self;
    if (ccself->pos) CcsPosition_Destruct(ccself->pos);
    CcNode_Destruct(self);
}
static const CcNodeType_t NodeSEM = {
    { sizeof(CcNodeSEM_t), "node_sem", CcNodeSEM_Destruct }, CcNode_Deletable
};
const CcObjectType_t * node_sem = (const CcObjectType_t *)&NodeSEM;

static void
CcNodeRSLV_Destruct(CcObject_t * self)
{
    CcNodeRSLV_t * ccself = (CcNodeRSLV_t *)self;
    if (ccself->pos) CcsPosition_Destruct(ccself->pos);
    CcNode_Destruct(self);
}
static const CcNodeType_t NodeRSLV = {
    { sizeof(CcNodeRSLV_t), "node_rslv", CcNodeRSLV_Destruct },
    CcNode_Deletable
};
const CcObjectType_t * node_rslv = (const CcObjectType_t *)&NodeRSLV;
CcNode_t *
CcNodeRslvP(int line, CcsPosition_t * pos)
{
    CcNodeRSLV_t * self = (CcNodeRSLV_t *)CcNode(&NodeRSLV, line);
    self->pos = pos;
    return (CcNode_t *)self;
}

void
CcNode_SetPosition(CcNode_t * self, CcsPosition_t * pos)
{
    if (self->base.type == node_nt) {
	((CcNodeNT_t *)self)->pos = pos;
    } else if (self->base.type == node_t) {
	((CcNodeT_t *)self)->pos = pos;
    } else if (self->base.type == node_wt) {
	((CcNodeWT_t *)self)->pos = pos;
    } else if (self->base.type == node_sem) {
	((CcNodeSEM_t *)self)->pos = pos;
    } else if (self->base.type == node_rslv) {
	((CcNodeRSLV_t *)self)->pos = pos;
    } else {
	CcsAssert(0);
    }
}
