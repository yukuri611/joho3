parameter INIT_G__this_pc_0 = 32'h0;

function[31:0] _init_reg_G__this_pc;
input[31:0] m_ID;
case(m_ID)

    32'd0 : _init_reg_G__this_pc = INIT_G__this_pc_0;
    default : _init_reg_G__this_pc = 0;

endcase
endfunction /*_init_reg_G__this_pc*/
