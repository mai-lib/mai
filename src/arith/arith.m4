m4_changecom(`M4', `')
m4_changequote(`{{', `}}')

M4 bind the first argument (name) as an element of the second (quoted list)
M4 and expand the third argument (quotation) for each one in order

m4_define({{shift3}}, {{m4_shift(m4_shift(m4_shift($1)))}})
m4_define({{_foreach}}, {{
  m4_ifelse({{$#}},{{3}},,{{
    m4_define({{$1}}, {{$3}})
    $2
    $0({{$1}},{{$2}},shift3({{$@}}))
  }})
}})
m4_define({{foreach}}, {{m4_pushdef({{$1}})_foreach({{$1}},{{$3}},$2,)m4_popdef({{$1}})}})

M4 usual list routines
m4_define({{car}}, {{$1}})
m4_define({{cdr}}, {{m4_shift($@)}})
m4_define({{cadr}}, {{car(cdr($@))}})

M4 token-pasting
m4_define({{paste}}, {{m4_ifelse({{$#}},1,{{$1}},{{$1_{{}}$0(m4_shift($@))}})}})

M4 diversion wrapper
m4_define({{stash}},{{m4_divert($1){{}}$2{{}}m4_divert}})

M4 names of things
m4_define({{make_type}}, {{r$1_t}})
m4_define({{make_type_obj}}, {{r_type_$1}})
m4_define({{check_na}}, {{$2_na($1)}})
m4_define({{make_na}}, {{r$1_na}})
m4_define({{make_min}}, {{r$1_min}})
m4_define({{make_max}}, {{r$1_max}})

M4 static arithmetic promotion: expand to the argument found first in `types`
M4 FIXME should be ifelse(car($3), `$1', match one, car($3), `$2', match two, no match)
m4_define({{_make_rtype}}, {{m4_ifelse({{$1}}, car($3), {{$1}},
{{m4_ifelse({{$2}}, car($3), {{$2}},
  {{$0({{$1}}, {{$2}}, {{cdr($3)}})}})}})}})
m4_define({{make_rtype}}, {{_make_rtype($1, $2, {{scalar_types}})}})

M4 convenient synonyms
m4_define({{xtype_t}}, {{make_type(xtype)}})
m4_define({{ytype_t}}, {{make_type(ytype)}})
m4_define({{rtype}}, {{make_rtype(xtype, ytype)}})
m4_define({{rtype_t}}, {{make_type(rtype)}})
m4_define({{op_name}}, {{car(op_spec)}})
m4_define({{op_sym}}, {{cadr(op_spec)}})

M4 m4_define({{op_binary}}, {{$1 cadr(op_spec) $2}})
m4_define({{op_binary}}, {{m4_ifelse(op_sym,{{}},
  {{op_name{{}}($1, $2)}},
  {{$1 op_sym $2}})}})

M4 m4_define({{op_unary}}, {{cadr(op_spec) $1}})
m4_define({{op_unary}}, {{m4_ifelse(op_sym,{{}},
  {{op_name{{}}($1)}},
  {{op_sym $1}})}})

m4_define({{op_conv}}, {{$2_conv($1)}})
M4 double ops have no overflow
m4_define({{op_xflow}}, {{m4_ifelse(rtype,{{double}},0,
  {{op_name{{}}_xflow($1, $2)}})}})
m4_define({{conv_func}}, {{paste({{arith}}, {{conv}}, xtype, {{to}}, ytype)}})
m4_define({{binary_func}}, {{paste({{arith}}, op_name, xtype, ytype)}})
m4_define({{unary_func}}, {{paste({{arith}}, op_name, xtype)}})

M4 loops
m4_define({{for_ops}}, {{foreach(op_spec, {{$1}}, {{$2}})}})
m4_define({{for_type}}, {{foreach(xtype, {{$1}}, {{$2}})}})
m4_define({{for_types}}, {{
  foreach(xtype, {{$1}}, {{
  foreach(ytype, {{$1}}, {{
  $2
  }})
  }})
}})

M4 configuration
m4_define({{element_types}}, {{double, int, boolean, ptr}})
m4_define({{scalar_types}}, {{double, int, boolean}})
m4_define({{numeric_types}}, {{double, int}})

M4 on numeric types, return promoted
m4_define({{arith_ops}}, {{{{add, +}}, {{sub, -}},
  {{mul, *}}, {{div, /}}, {{pow}}}})
M4 on numeric types, return same
m4_define({{arith_unary_ops}}, {{{{neg, -}}}})
M4 on numeric types, return same, no analogous symbol
M4 use cadr(op_spec) for identity instead
m4_define({{arith_reduce_ops}}, {{{{add, 0}}, {{mul, 1}}}})

M4 on numeric types, return boolean
m4_define({{rela_ops}}, {{{{lth, <}}, {{lte, <=}}, {{gth, >}}, {{gte, >=}}}})

M4 on scalar types, return boolean
m4_define({{eql_ops}}, {{{{eql, ==}}, {{neq, !=}}}})
M4 on scalar types, return boolean
M4 none for now
M4 m4_define({{eql_unary_ops}}, {{{{something}}}})

M4 on booleans only
m4_define({{bool_ops}}, {{{{and, &}}, {{or, |}}}})
M4 on booleans only
m4_define({{bool_unary_ops}}, {{{{not, !}}}})
M4 on booleans only, etc, etc
m4_define({{bool_reduce_ops}}, {{{{and, true}}, {{or, false}}}})

M4 on doubles only
m4_define({{math_ops}}, {{{{atan2}}}})
m4_define({{math_unary_ops}}, {{{{sqrt}}, {{sin}}, {{cos}}, {{tan}},
                                {{asin}}, {{acos}}, {{atan}},
                                {{log}}, {{exp}}, {{round}}}})
