module System = EsyLib.System;

print_endline("Hello from EsyBash!");


/* let brackets = Str.regexp("\\\\"); */
/* let testReplace = (path: string) => { */
/*     let replacedBrackets = Str.global_replace(brackets, "/", path); */
/*     replacedBrackets */
/* } */

let getCygwinUtilityPath = () => {
  open Run;
  let program = Sys.argv[0];
  let%bind program = realpath(v(program));
  let basedir = Fpath.parent(program);
  let resolution =
      EsyLib.NodeResolution.resolve(
        "../../../../node_modules/esy-bash/package.json",
        basedir,
      );

  switch%bind (Run.coerceFrmMsgOnly(resolution)) {
    | Some(path) => Ok(Fpath.parent(path))
    | None => Error(`Msg("bad stuff happened"))
  };
};

let getCygPath = () => {
    open Run;
    let%bind rootPath = getCygwinUtilityPath();
    Ok(Fpath.(rootPath / ".cygwin" / "bin" / "cygpath.exe"));
};

/**
* Wrapper around the cygPath utility
*/
/* let cygPath = (path) => { */
/*     open Run; */
/*     let%bind resolvedPath = getCygwinUtilityPath(".cygwin"); */
/*     print_endline("resolvedPath: " ++ resolvedPath); */
/*     Ok(resolvedPath) */
/* }; */

/**
* Helper utility to normalize paths to a cygwin style,
* ie, "C:\temp" -> "/cygdrive/c/temp"
* On non-Windows platforms, this is a noop
*/
let normalizePathForCygwin = (path) => {
    print_endline("NORMALIZE_PATH_FOR_CYGWIN")
    open Run;
    switch (System.host) {
        | System.Windows => {
            let%bind rootPath = getCygPath();
            print_endline("CYGPATH: " ++ Fpath.to_string(rootPath));
            let ic = Unix.open_process_in(Fpath.to_string(rootPath) ++ " \"" ++ path ++ " \"")
            let result = String.trim(input_line(ic));
            let () = close_in(ic);
            Ok(result);
        };
        | _ => Ok(path)
    }
};

let test = () => {
    open Run;
    /* let%bind result = getCygwinUtilityPath(); */
    /* let%bind cygPath = getCygPath(); */
    let%bind normalizedPath = normalizePathForCygwin("C:\\temp\\temp2");
    /* print_endline("GETTING CYGGY: " ++ Fpath.to_string(result)); */
    /* print_endline("CYGPATHY: " ++ Fpath.to_string(cygPath)); */
    print_endline("NORMALIED PATH: " ++ normalizedPath);
    Ok(normalizedPath);
};

test();
