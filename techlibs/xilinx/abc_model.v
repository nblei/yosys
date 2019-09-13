/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Clifford Wolf <clifford@clifford.at>
 *                2019  Eddie Hung    <eddie@fpgeh.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

// ============================================================================

// Box containing MUXF7.[AB] + MUXF8,
//   Necessary to make these an atomic unit so that
//   ABC cannot optimise just one of the MUXF7 away
//   and expect to save on its delay
(* abc_box_id = 3, lib_whitebox *)
module \$__XILINX_MUXF78 (output O, input I0, I1, I2, I3, S0, S1);
  assign O = S1 ? (S0 ? I3 : I2)
                : (S0 ? I1 : I0);
endmodule

// Box to emulate comb/seq behaviour of RAMD{32,64} and SRL{16,32}
//   Necessary since RAMD* and SRL* have both combinatorial (i.e.
//   same-cycle read operation) and sequential (write operation
//   is only committed on the next clock edge).
//   To model the combinatorial path, such cells have to be split
//   into comb and seq parts, with this box modelling only the former.
(* abc_box_id=2000 *)
module \$__ABC_LUT6 (input A, input [5:0] S, output Y);
endmodule
// Box to emulate comb/seq behaviour of RAMD128
(* abc_box_id=2001 *)
module \$__ABC_LUT7 (input A, input [6:0] S, output Y);
endmodule

(* abc_box_id=2100 *)
module \$__ABC_DSP48E1_MULT_P_MUX (input Aq, ADq, Bq, Cq, Dq, Mq, input [47:0] P, input Pq, output [47:0] O);
endmodule
(* abc_box_id=2101 *)
module \$__ABC_DSP48E1_MULT_PCOUT_MUX (input Aq, ADq, Bq, Cq, Dq, Mq, input [47:0] P, input Pq, output [47:0] O);
endmodule

// Box used to represent the comb/seq behaviour of DSP48E1
//   abc_map.v is responsible for disconnecting inputs to
//   the combinatorial DSP48E1 model by a register (e.g.
//   disconnecting A when AREG, MREG or PREG is enabled)
(* abc_box_id=3000 *)
module \$__ABC_DSP48E1_MULT (
    output [29:0] ACOUT,
    output [17:0] BCOUT,
    output reg CARRYCASCOUT,
    output reg [3:0] CARRYOUT,
    output reg MULTSIGNOUT,
    output OVERFLOW,
    output reg signed [47:0] P,
    output PATTERNBDETECT,
    output PATTERNDETECT,
    output [47:0] PCOUT,
    output UNDERFLOW,
    input signed [29:0] A,
    input [29:0] ACIN,
    input [3:0] ALUMODE,
    input signed [17:0] B,
    input [17:0] BCIN,
    input [47:0] C,
    input CARRYCASCIN,
    input CARRYIN,
    input [2:0] CARRYINSEL,
    input CEA1,
    input CEA2,
    input CEAD,
    input CEALUMODE,
    input CEB1,
    input CEB2,
    input CEC,
    input CECARRYIN,
    input CECTRL,
    input CED,
    input CEINMODE,
    input CEM,
    input CEP,
    input CLK,
    input [24:0] D,
    input [4:0] INMODE,
    input MULTSIGNIN,
    input [6:0] OPMODE,
    input [47:0] PCIN,
    input RSTA,
    input RSTALLCARRYIN,
    input RSTALUMODE,
    input RSTB,
    input RSTC,
    input RSTCTRL,
    input RSTD,
    input RSTINMODE,
    input RSTM,
    input RSTP
);
    parameter integer ACASCREG = 1;
    parameter integer ADREG = 1;
    parameter integer ALUMODEREG = 1;
    parameter integer AREG = 1;
    parameter AUTORESET_PATDET = "NO_RESET";
    parameter A_INPUT = "DIRECT";
    parameter integer BCASCREG = 1;
    parameter integer BREG = 1;
    parameter B_INPUT = "DIRECT";
    parameter integer CARRYINREG = 1;
    parameter integer CARRYINSELREG = 1;
    parameter integer CREG = 1;
    parameter integer DREG = 1;
    parameter integer INMODEREG = 1;
    parameter integer MREG = 1;
    parameter integer OPMODEREG = 1;
    parameter integer PREG = 1;
    parameter SEL_MASK = "MASK";
    parameter SEL_PATTERN = "PATTERN";
    parameter USE_DPORT = "FALSE";
    parameter USE_MULT = "MULTIPLY";
    parameter USE_PATTERN_DETECT = "NO_PATDET";
    parameter USE_SIMD = "ONE48";
    parameter [47:0] MASK = 48'h3FFFFFFFFFFF;
    parameter [47:0] PATTERN = 48'h000000000000;
    parameter [3:0] IS_ALUMODE_INVERTED = 4'b0;
    parameter [0:0] IS_CARRYIN_INVERTED = 1'b0;
    parameter [0:0] IS_CLK_INVERTED = 1'b0;
    parameter [4:0] IS_INMODE_INVERTED = 5'b0;
    parameter [6:0] IS_OPMODE_INVERTED = 7'b0;
endmodule


