// Copyright 2014 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <cstddef>
#include <map>
#include <tuple>

#include <rangeset/rangesizeset.h>

#include "Common/Arm64Emitter.h"

#include "Core/PowerPC/CPUCoreBase.h"
#include "Core/PowerPC/JitArm64/JitArm64Cache.h"
#include "Core/PowerPC/JitArm64/JitArm64_RegCache.h"
#include "Core/PowerPC/JitArmCommon/BackPatch.h"
#include "Core/PowerPC/JitCommon/JitAsmCommon.h"
#include "Core/PowerPC/JitCommon/JitBase.h"
#include "Core/PowerPC/PPCAnalyst.h"

class JitArm64 : public JitBase, public Arm64Gen::ARM64CodeBlock, public CommonAsmRoutinesBase
{
public:
  JitArm64();
  ~JitArm64() override;

  void Init() override;
  void Shutdown() override;

  JitBaseBlockCache* GetBlockCache() override { return &blocks; }
  bool IsInCodeSpace(const u8* ptr) const { return IsInSpace(ptr); }
  bool HandleFault(uintptr_t access_address, SContext* ctx) override;
  void DoBacktrace(uintptr_t access_address, SContext* ctx);
  bool HandleStackFault() override;
  bool HandleFastmemFault(SContext* ctx);

  void ClearCache() override;

  CommonAsmRoutinesBase* GetAsmRoutines() override { return this; }
  void Run() override;
  void SingleStep() override;

  void Jit(u32 em_address) override;
  void Jit(u32 em_address, bool clear_cache_and_retry_on_failure);

  const char* GetName() const override { return "JITARM64"; }

  // OPCODES
  using Instruction = void (JitArm64::*)(UGeckoInstruction);
  void FallBackToInterpreter(UGeckoInstruction inst);
  void DoNothing(UGeckoInstruction inst);
  void HLEFunction(u32 hook_index);

  void DynaRunTable4(UGeckoInstruction inst);
  void DynaRunTable19(UGeckoInstruction inst);
  void DynaRunTable31(UGeckoInstruction inst);
  void DynaRunTable59(UGeckoInstruction inst);
  void DynaRunTable63(UGeckoInstruction inst);

  // Force break
  void Break(UGeckoInstruction inst);

  // Branch
  void sc(UGeckoInstruction inst);
  void rfi(UGeckoInstruction inst);
  void bx(UGeckoInstruction inst);
  void bcx(UGeckoInstruction inst);
  void bcctrx(UGeckoInstruction inst);
  void bclrx(UGeckoInstruction inst);

  // Integer
  void arith_imm(UGeckoInstruction inst);
  void boolX(UGeckoInstruction inst);
  void addx(UGeckoInstruction inst);
  void addix(UGeckoInstruction inst);
  void extsXx(UGeckoInstruction inst);
  void cntlzwx(UGeckoInstruction inst);
  void negx(UGeckoInstruction inst);
  void cmp(UGeckoInstruction inst);
  void cmpl(UGeckoInstruction inst);
  void cmpi(UGeckoInstruction inst);
  void cmpli(UGeckoInstruction inst);
  void rlwinmx(UGeckoInstruction inst);
  void rlwnmx(UGeckoInstruction inst);
  void srawix(UGeckoInstruction inst);
  void mullwx(UGeckoInstruction inst);
  void mulhwx(UGeckoInstruction inst);
  void mulhwux(UGeckoInstruction inst);
  void addic(UGeckoInstruction inst);
  void mulli(UGeckoInstruction inst);
  void addzex(UGeckoInstruction inst);
  void divwx(UGeckoInstruction inst);
  void subfx(UGeckoInstruction inst);
  void addcx(UGeckoInstruction inst);
  void slwx(UGeckoInstruction inst);
  void srwx(UGeckoInstruction inst);
  void srawx(UGeckoInstruction inst);
  void rlwimix(UGeckoInstruction inst);
  void subfex(UGeckoInstruction inst);
  void subfzex(UGeckoInstruction inst);
  void subfcx(UGeckoInstruction inst);
  void subfic(UGeckoInstruction inst);
  void addex(UGeckoInstruction inst);
  void divwux(UGeckoInstruction inst);

  // System Registers
  void mtmsr(UGeckoInstruction inst);
  void mfmsr(UGeckoInstruction inst);
  void mcrf(UGeckoInstruction inst);
  void mcrxr(UGeckoInstruction inst);
  void mfsr(UGeckoInstruction inst);
  void mtsr(UGeckoInstruction inst);
  void mfsrin(UGeckoInstruction inst);
  void mtsrin(UGeckoInstruction inst);
  void twx(UGeckoInstruction inst);
  void mfspr(UGeckoInstruction inst);
  void mftb(UGeckoInstruction inst);
  void mtspr(UGeckoInstruction inst);
  void crXXX(UGeckoInstruction inst);
  void mfcr(UGeckoInstruction inst);
  void mtcrf(UGeckoInstruction inst);
  void mcrfs(UGeckoInstruction inst);
  void mffsx(UGeckoInstruction inst);
  void mtfsb0x(UGeckoInstruction inst);
  void mtfsb1x(UGeckoInstruction inst);
  void mtfsfix(UGeckoInstruction inst);
  void mtfsfx(UGeckoInstruction inst);

  // LoadStore
  void lXX(UGeckoInstruction inst);
  void stX(UGeckoInstruction inst);
  void lmw(UGeckoInstruction inst);
  void stmw(UGeckoInstruction inst);
  void dcbx(UGeckoInstruction inst);
  void dcbt(UGeckoInstruction inst);
  void dcbz(UGeckoInstruction inst);
  void eieio(UGeckoInstruction inst);

  // LoadStore floating point
  void lfXX(UGeckoInstruction inst);
  void stfXX(UGeckoInstruction inst);

  // Floating point
  void fp_arith(UGeckoInstruction inst);
  void fp_logic(UGeckoInstruction inst);
  void fselx(UGeckoInstruction inst);
  void fcmpX(UGeckoInstruction inst);
  void frspx(UGeckoInstruction inst);
  void fctiwx(UGeckoInstruction inst);
  void fresx(UGeckoInstruction inst);
  void frsqrtex(UGeckoInstruction inst);

  // Paired
  void ps_mergeXX(UGeckoInstruction inst);
  void ps_arith(UGeckoInstruction inst);
  void ps_sel(UGeckoInstruction inst);
  void ps_sumX(UGeckoInstruction inst);
  void ps_res(UGeckoInstruction inst);
  void ps_rsqrte(UGeckoInstruction inst);
  void ps_cmpXX(UGeckoInstruction inst);

  // Loadstore paired
  void psq_lXX(UGeckoInstruction inst);
  void psq_stXX(UGeckoInstruction inst);

  void ConvertDoubleToSingleLower(size_t guest_reg, Arm64Gen::ARM64Reg dest_reg,
                                  Arm64Gen::ARM64Reg src_reg);
  void ConvertDoubleToSinglePair(size_t guest_reg, Arm64Gen::ARM64Reg dest_reg,
                                 Arm64Gen::ARM64Reg src_reg);
  void ConvertSingleToDoubleLower(size_t guest_reg, Arm64Gen::ARM64Reg dest_reg,
                                  Arm64Gen::ARM64Reg src_reg,
                                  Arm64Gen::ARM64Reg scratch_reg = Arm64Gen::ARM64Reg::INVALID_REG);
  void ConvertSingleToDoublePair(size_t guest_reg, Arm64Gen::ARM64Reg dest_reg,
                                 Arm64Gen::ARM64Reg src_reg,
                                 Arm64Gen::ARM64Reg scratch_reg = Arm64Gen::ARM64Reg::INVALID_REG);

  void FloatCompare(UGeckoInstruction inst, bool upper = false);

  // temp_gpr can be INVALID_REG if single is true
  void EmitQuietNaNBitConstant(Arm64Gen::ARM64Reg dest_reg, bool single,
                               Arm64Gen::ARM64Reg temp_gpr);

  bool IsFPRStoreSafe(size_t guest_reg) const;

protected:
  struct FastmemArea
  {
    const u8* fastmem_code;
    const u8* slowmem_code;
  };

  void SetBlockLinkingEnabled(bool enabled);
  void SetOptimizationEnabled(bool enabled);

  void CompileInstruction(PPCAnalyst::CodeOp& op);

  bool HandleFunctionHooking(u32 address);

  // Simple functions to switch between near and far code emitting
  void SwitchToFarCode()
  {
    m_near_code = GetWritableCodePtr();
    m_near_code_end = GetWritableCodeEnd();
    m_near_code_write_failed = HasWriteFailed();
    SetCodePtrUnsafe(m_far_code.GetWritableCodePtr(), m_far_code.GetWritableCodeEnd(),
                     m_far_code.HasWriteFailed());
    AlignCode16();
    m_in_far_code = true;
  }

  void SwitchToNearCode()
  {
    m_far_code.SetCodePtrUnsafe(GetWritableCodePtr(), GetWritableCodeEnd(), HasWriteFailed());
    SetCodePtrUnsafe(m_near_code, m_near_code_end, m_near_code_write_failed);
    m_in_far_code = false;
  }

  bool IsInFarCode() const { return m_in_far_code; }

  // Dump a memory range of code
  void DumpCode(const u8* start, const u8* end);

  // This enum is used for selecting an implementation of EmitBackpatchRoutine.
  enum class MemAccessMode
  {
    // Always calls the slow C++ code. For performance reasons, should generally only be used if
    // the guest address is known in advance and IsOptimizableRAMAddress returns false for it.
    AlwaysSafe,
    // Only emits fast access code. Must only be used if the guest address is known in advance
    // and IsOptimizableRAMAddress returns true for it, otherwise Dolphin will likely crash!
    AlwaysUnsafe,
    // Best in most cases. If backpatching is possible (!emitting_routine && jo.fastmem_arena):
    // Tries to run fast access code, and if that fails, uses backpatching to replace the code
    // with a call to the slow C++ code. Otherwise: Checks whether the fast access code will work,
    // then branches to either the fast access code or the slow C++ code.
    Auto,
  };

  // This is the core routine for accessing emulated memory, with support for
  // many different kinds of loads and stores as well as fastmem/backpatching.
  //
  // Registers used:
  //
  //                 addr     scratch
  // Store:          X1       X0
  // Load:           X0
  // Zero 256:       X0       X30
  // Store float:    X1       Q0
  // Load float:     X0
  //
  // If mode == AlwaysUnsafe, the addr argument can be any register.
  // Otherwise it must be the register listed in the table above.
  //
  // Additional scratch registers are used in the following situations:
  //
  // emitting_routine && mode == Auto:                                            X2
  // emitting_routine && mode == Auto && !(flags & BackPatchInfo::FLAG_STORE):    X3
  // emitting_routine && mode != AlwaysSafe && !jo.fastmem_arena:                 X3
  // mode != AlwaysSafe && !jo.fastmem_arena:                                     X2
  // !emitting_routine && mode != AlwaysSafe && !jo.fastmem_arena:                X30
  // !emitting_routine && mode == Auto && jo.fastmem_arena:                       X30
  //
  // Furthermore, any callee-saved register which isn't marked in gprs_to_push/fprs_to_push
  // may be clobbered if mode != AlwaysUnsafe.
  void EmitBackpatchRoutine(u32 flags, MemAccessMode mode, Arm64Gen::ARM64Reg RS,
                            Arm64Gen::ARM64Reg addr, BitSet32 gprs_to_push = BitSet32(0),
                            BitSet32 fprs_to_push = BitSet32(0), bool emitting_routine = false);

  // Loadstore routines
  void SafeLoadToReg(u32 dest, s32 addr, s32 offsetReg, u32 flags, s32 offset, bool update);
  void SafeStoreFromReg(s32 dest, u32 value, s32 regOffset, u32 flags, s32 offset, bool update);
  // If lookup succeeds, writes upper 15 bits of physical address to addr_out. If not,
  // jumps to the returned FixupBranch. Clobbers tmp and the 17 lower bits of addr_out.
  Arm64Gen::FixupBranch BATAddressLookup(Arm64Gen::ARM64Reg addr_out, Arm64Gen::ARM64Reg addr_in,
                                         Arm64Gen::ARM64Reg tmp, const void* bat_table);
  Arm64Gen::FixupBranch CheckIfSafeAddress(Arm64Gen::ARM64Reg addr, Arm64Gen::ARM64Reg tmp1,
                                           Arm64Gen::ARM64Reg tmp2);

  bool DoJit(u32 em_address, JitBlock* b, u32 nextPC);

  void Trace();

  // Finds a free memory region and sets the near and far code emitters to point at that region.
  // Returns false if no free memory region can be found for either of the two.
  bool SetEmitterStateToFreeCodeRegion();

  void DoDownCount();
  void Cleanup();
  void ResetStack();
  void AllocStack();
  void FreeStack();

  void ResetFreeMemoryRanges();

  void IntializeSpeculativeConstants();

  // AsmRoutines
  void GenerateAsm();
  void GenerateCommonAsm();
  void GenerateFres();
  void GenerateFrsqrte();
  void GenerateConvertDoubleToSingle();
  void GenerateConvertSingleToDouble();
  void GenerateFPRF(bool single);
  void GenerateQuantizedLoads();
  void GenerateQuantizedStores();

  // Profiling
  void BeginTimeProfile(JitBlock* b);
  void EndTimeProfile(JitBlock* b);

  // Exits
  void WriteExit(u32 destination, bool LK = false, u32 exit_address_after_return = 0);
  void WriteExit(Arm64Gen::ARM64Reg dest, bool LK = false, u32 exit_address_after_return = 0);
  void WriteExceptionExit(u32 destination, bool only_external = false,
                          bool always_exception = false);
  void WriteExceptionExit(Arm64Gen::ARM64Reg dest, bool only_external = false,
                          bool always_exception = false);
  void WriteConditionalExceptionExit(int exception, u64 increment_sp_on_exit = 0);
  void WriteConditionalExceptionExit(int exception, Arm64Gen::ARM64Reg temp_gpr,
                                     Arm64Gen::ARM64Reg temp_fpr = Arm64Gen::ARM64Reg::INVALID_REG,
                                     u64 increment_sp_on_exit = 0);
  void FakeLKExit(u32 exit_address_after_return);
  void WriteBLRExit(Arm64Gen::ARM64Reg dest);

  Arm64Gen::FixupBranch JumpIfCRFieldBit(int field, int bit, bool jump_if_set);
  void FixGTBeforeSettingCRFieldBit(Arm64Gen::ARM64Reg reg);
  void UpdateFPExceptionSummary(Arm64Gen::ARM64Reg fpscr);
  void UpdateRoundingMode();

  void ComputeRC0(Arm64Gen::ARM64Reg reg);
  void ComputeRC0(u64 imm);
  void ComputeCarry(Arm64Gen::ARM64Reg reg);  // reg must contain 0 or 1
  void ComputeCarry(bool carry);
  void ComputeCarry();
  void LoadCarry();
  void FlushCarry();

  void reg_imm(u32 d, u32 a, u32 value, u32 (*do_op)(u32, u32),
               void (ARM64XEmitter::*op)(Arm64Gen::ARM64Reg, Arm64Gen::ARM64Reg, u64,
                                         Arm64Gen::ARM64Reg),
               bool Rc = false);
  bool MultiplyImmediate(u32 imm, int a, int d, bool rc);

  void SetFPRFIfNeeded(bool single, Arm64Gen::ARM64Reg reg);
  void Force25BitPrecision(Arm64Gen::ARM64Reg output, Arm64Gen::ARM64Reg input);

  // <Fastmem fault location, slowmem handler location>
  std::map<const u8*, FastmemArea> m_fault_to_handler{};
  Arm64GPRCache gpr;
  Arm64FPRCache fpr;

  JitArm64BlockCache blocks{*this};

  Arm64Gen::ARM64FloatEmitter m_float_emit;

  Arm64Gen::ARM64CodeBlock m_far_code;
  bool m_in_far_code = false;

  // Backed up when we switch to far code.
  u8* m_near_code = nullptr;
  u8* m_near_code_end = nullptr;
  bool m_near_code_write_failed = false;

  bool m_enable_blr_optimization = false;
  bool m_cleanup_after_stackfault = false;
  u8* m_stack_base = nullptr;
  u8* m_stack_pointer = nullptr;
  u8* m_saved_stack_pointer = nullptr;

  HyoutaUtilities::RangeSizeSet<u8*> m_free_ranges_near;
  HyoutaUtilities::RangeSizeSet<u8*> m_free_ranges_far;
};
