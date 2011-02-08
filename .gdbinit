define astloc
  printf "%s: %s:%d.%d-%d.%d\n", (('ast::Ast'*) $arg0)->node_type().c_str(), (('ast::Ast'*) $arg0)->location_get().begin.filename.str_->c_str(), (('ast::Ast'*) $arg0)->location_get().begin.line, (('ast::Ast'*) $arg0)->location_get().begin.column, (('ast::Ast'*) $arg0)->location_get().end.line, (('ast::Ast'*) $arg0)->location_get().end.column
end
document astloc
astloc EXP
URBI specific debug function.

Display the type of the AST node and the location in the urbiscript.
end
