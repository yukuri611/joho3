# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "M_ID" -parent ${Page_0}


}

proc update_PARAM_VALUE.M_ID { PARAM_VALUE.M_ID } {
	# Procedure called to update M_ID when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.M_ID { PARAM_VALUE.M_ID } {
	# Procedure called to validate M_ID
	return true
}


proc update_MODELPARAM_VALUE.M_ID { MODELPARAM_VALUE.M_ID PARAM_VALUE.M_ID } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.M_ID}] ${MODELPARAM_VALUE.M_ID}
}

