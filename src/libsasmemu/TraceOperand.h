#ifndef SASM_TRACEOPERAND_H_INCLUDED
#define SASM_TRACEOPERAND_H_INCLUDED

#include "Types.h"
#include "Disassembler.h"
#include "C64MachineState.h"

#include <utility>
#include <map>
#include <set>
#include <vector>
#include <deque>

#define SASM_DEBUG_TRACESTATE 1

namespace SASM {

struct MemAccessMap;
struct TraceNode;
class C64MachineState;

template<typename T>
struct NodePtrT {
                            NodePtrT() : m_Node(NULL) {
                            }
                            NodePtrT(T* node) : m_Node(node) {
                              if (m_Node) {
                                m_Node->addRef();
                              }
                            }
                            NodePtrT(NodePtrT const& rhs) : m_Node(rhs.m_Node) {
                              if (m_Node)  {
                                m_Node->addRef();
                              }
                            }
                            ~NodePtrT() {
                              clear();
                            }
  inline void               clear() {
                              if (m_Node) {
                                m_Node->release();
                                m_Node = NULL;
                              }
                            }
  inline void               set(T* node) {
                              if (node) {
                                node->addRef();
                              }
                              clear();
                              m_Node = node;
                            }
  inline T*                 get()   const { return m_Node; }
  inline int                refCount() const {
                              if (m_Node) {
                                return m_Node->refCount();
                              } else {
                                return 0;
                              }
                            }
  NodePtrT&                 operator= (T* node) {
                              set(node);
                              return *this;
                            }
  NodePtrT&                 operator= (T& node) {
                              set(&node);
                              return *this;
                            }
  NodePtrT&                 operator= (NodePtrT const& rhs) {
                              if (&rhs != this) {
                                set(rhs.m_Node);
                              }
                              return *this;
                            }
  bool                      operator< (NodePtrT const& rhs) const {
                              return this->m_Node < rhs.m_Node;
                            }
private:
  T*                        m_Node;
};

typedef NodePtrT<TraceNode> TraceNodePtr;

struct TraceNode {
  static TraceNodePtr       s_Unresolvable;
  static TraceNodePtr       s_Null;

  TraceNodePtr              m_Left;
  TraceNodePtr              m_Right;
  int                       m_Address;
  int                       m_RefCount;
  mutable int               m_MaxDepth;
  mutable int               m_LeafNodes;

#if SASM_DEBUG_TRACESTATE
  static int64_t            s_CreateCount;
  static int64_t            s_AliveCount;
  static int64_t            s_MaxAliveCount;
#endif

  static inline TraceNode*  create(int addr, TraceNode* left, TraceNode* right) {
                              if (right != NULL && left == NULL) {
                                // swap
                                return new TraceNode(addr, right, left);
                              } else {
                                return new TraceNode(addr, left, right);
                              }
                            }
  static inline TraceNode*  getUnresolvable() {
                              if (s_Unresolvable.get() == NULL) {
                                s_Unresolvable = new TraceNode(-1, NULL, NULL);
                                assert(s_Unresolvable.get()->refCount() == 1);
                              }
                              return s_Unresolvable.get();
                            }
  static inline TraceNode*  getNull() {
                              if (s_Null.get() == NULL) {
                                s_Null = new TraceNode(0, NULL, NULL);
                                assert(s_Null.get()->refCount() == 1);
                              }
                              return s_Null.get();
                            }
  inline int                address() const {
                              return m_Address;
                            }
  inline TraceNode*         left() const {
                              return m_Left.get();
                            }
  inline TraceNode*         right() const {
                              return m_Right.get();
                            }
  int                       leafNodes() const;
  inline int                maxdepth() const {
                              if (!m_MaxDepth) {
                                int left  = m_Left.get()  ? m_Left.get()->maxdepth()  : 0;
                                int right = m_Right.get() ? m_Right.get()->maxdepth() : 0;
                                m_MaxDepth = (left >= right ? left : right) + 1;
                              }
                              return m_MaxDepth;
                            }
  void                      addRef() {
                              ++m_RefCount;
                            }
  bool                      isResolvable(C64MachineState& emu, int addr, int regionsize) const {
                              if (m_Left.get()) {
                                if (m_Right.get()) {
                                  if (!m_Right.get()->isResolvable(emu, addr, regionsize)) {
                                    return false;
                                  }
                                }
                                return m_Left.get()->isResolvable(emu, addr, regionsize);
                              }
                              return m_Address >= 0;
                            }
  void                      getLeafNodes(std::set<int>& nodes) const {
                              if (m_Left.get())
                              {
                                m_Left.get()->getLeafNodes(nodes);
                                if (m_Right.get()) 
                                {
                                  m_Right.get()->getLeafNodes(nodes);
                                }
                              }
                              else
                              {
                                nodes.insert(m_Address);
                              }
                            }
private:
                            TraceNode(int addr, TraceNode* left, TraceNode* right) : m_Address(addr), m_RefCount(0), m_MaxDepth(0), m_LeafNodes(0) {
#if SASM_DEBUG_TRACESTATE
                              ++s_CreateCount;
                              ++s_AliveCount;
                              s_MaxAliveCount = s_AliveCount > s_MaxAliveCount ? s_AliveCount : s_MaxAliveCount;
#endif
                              m_Left = left;
                              if (right) {
                                assert(left);
                              }
                              m_Right = right;
                            }
                            ~TraceNode() {
#if SASM_DEBUG_TRACESTATE
                              --s_AliveCount;
#endif
                            }
  inline void               release() {
                              assert(m_RefCount > 0);
                              --m_RefCount;
                              if (m_RefCount == 0) {
                                delete this;
                              }
                            }
  inline int                refCount() const {
                              return m_RefCount;
                            }

  friend TraceNodePtr;
  friend struct OperandTraceState;
};

struct AddressOperandTrace {
  TraceNodePtr              m_Lo;
  TraceNodePtr              m_Hi;
  int                       m_OpAddr;
  int                       m_RefCount;

                            AddressOperandTrace(int opaddr, TraceNode* lo, TraceNode* hi) : m_OpAddr(opaddr), m_RefCount(0) {
                              assert((lo == NULL && hi == NULL) || (lo != NULL && hi != NULL));
                              if (lo) {
                                m_Lo = lo;
                                m_Hi = hi;
                              }
                            }
                            AddressOperandTrace(int opaddr, TraceNode* lo) : m_OpAddr(opaddr), m_RefCount(0) {
                              if (lo) {
                                m_Lo = lo;
                                m_Hi = TraceNode::getNull();
                              }
                            }
                            ~AddressOperandTrace() {
                              assert(m_RefCount == 0);
                            }
  int                       refCount() const {
                              return m_RefCount;
                            }
  void                      addRef() {
                              ++m_RefCount;
                            }
  void                      release() {
                              assert(m_RefCount > 0);
                              --m_RefCount;
                              if (m_RefCount == 0) {
                                delete this;
                              }
                            }
  bool                      isResolvable(C64MachineState& emu, int addr, int regionsize);
  bool                      isNoTrace() {
                              return m_Lo.get() == NULL && m_Hi.get() == NULL;
                            }
};

typedef NodePtrT<AddressOperandTrace> AddressOperandTracePtr;

struct RelocationTableDef {
  int   m_LoBytes;
  int   m_HiBytes;
  int   m_Size;

  RelocationTableDef(int lobytes, int hibytes, int size) : m_LoBytes(lobytes), m_HiBytes(hibytes), m_Size(size) {
  }
};

struct OperandTraceState {
                            OperandTraceState();
                            ~OperandTraceState();
  void                      init();
  void                      cleanup();
  void                      dumpStats();
  TraceNode*                lookupOrigin(int addr);
  void                      addRelocAddr(std::pair<int, int> lohi);
  void                      addTraceNodePair(TraceNode* lo, TraceNode* hi);
  void                      enableOperandTracing(C64MachineState& emu, int relocRangeStart, int relocRangeSize);
  void                      buildRelocationTables(C64MachineState& emu, int relocRangeStart, int relocRangeSize, bool bridgeGaps = true);
  std::vector<RelocationTableDef>&
                            relocationTables() { return m_RelocationTables; }
  int                       removeOverlappingRelocationTables(int lobytes, int hibytes, int count);
  int                       removeOverlappingRelocationTables(Disassembler::AddressTable* tbl);

  TraceNodePtr              m_A;
  TraceNodePtr              m_X;
  TraceNodePtr              m_Y;

  TraceNodePtr*             m_CurrentTraces;  // [65536];
  std::map<int, AddressOperandTracePtr>
                            m_Unresolvable;
  std::set<std::pair<int, int> >
                            m_RelocPairs;
  std::set<std::pair<TraceNodePtr, TraceNodePtr> >
                            m_TraceNodePairs;
  std::vector<RelocationTableDef>
                            m_RelocationTables;
  std::deque<TraceNodePtr>  m_Stack;
  std::set<int>             m_IncompleteOperands;
  bool                      m_DumpRelocationTables;
};

} // namespace SASM

#endif
