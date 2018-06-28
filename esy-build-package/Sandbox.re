module Path = EsyLib.Path;

type pattern =
  | Subpath(string)
  | Regex(string);

type config = {allowWrite: list(pattern)};

module Darwin = {
  let renderConfig = config => {
    open Sexp;
    let v = x => Value(L(x));
    let renderAllowWrite =
      List.map(
        fun
        | Subpath(p) =>
          v([I("allow"), I("file-write*"), L([I("subpath"), S(p)])])
        | Regex(p) =>
          v([I("allow"), I("file-write*"), L([I("regex"), S(p)])]),
      );
    let doc =
      [
        v([I("version"), N(1.0)]),
        v([I("allow"), I("default")]),
        v([I("deny"), I("file-write*"), L([I("subpath"), S("/")])]),
        v([
          I("allow"),
          I("file-write*"),
          L([I("literal"), S("/dev/null")]),
        ]),
      ]
      @ renderAllowWrite(config.allowWrite);
    render(doc);
  };
  let sandboxExec = config => {
    open Run;
    let configData = renderConfig(config);
    let%bind configFilename = putTempFile(configData);
    let exec = (~err, ~env, command) => {
      open Bos.OS.Cmd;
      let sandboxCommand =
        Bos.Cmd.of_list([
          "sandbox-exec",
          "-f",
          Path.to_string(configFilename),
        ]);
      let command = Bos.Cmd.(sandboxCommand %% command);
      run_io(~env, ~err, command);
    };
    Ok(exec);
  };
};

let regexp = Str.regexp("E:\\\\");
let regexp2 = Str.regexp("C:\\\\");
let brackets = Str.regexp("\\\\")

let normalizePath = (path: string) => {
    let replaceDrive = Str.global_replace(regexp, "/cygdrive/e/", path);
    let replaceDrive2 = Str.global_replace(regexp2, "/cygdrive/c/", replaceDrive);
    let replacedBrackets = Str.global_replace(brackets, "/", replaceDrive2);
    replacedBrackets
}

module Windows = {
    let sandboxExec = config => {
        open Run;
        let exec = (~err, ~env, command) => {
            open Bos.OS.Cmd;
            let commands = Bos.Cmd.to_list(command);
            let normalizedCommands = List.map(normalizePath, commands)
            let cygwinCommand =
                Bos.Cmd.of_list([
                    "node",
                    "E:/esy-bash/bin/esy-bash.js",
                    ...normalizedCommands,
                ]);
                /* Bos.Cmd.of_list([ */
                /*     ...normalizedCommands, */
                /* ]); */
                /* Bos.Cmd.of_list([ */
                /*     "/usr/bin/bash", */
                /*     "--version", */
                /* ]); */
            print_endline("Running command4: " ++ Bos.Cmd.to_string(cygwinCommand));
            run_io(cygwinCommand);
        };
        Ok(exec);
    };
}

module NoSandbox = {
  let sandboxExec = _config => {
    let exec = (~err, ~env, command) => {
      Printf.printf("NoSandbox::sandboxExec command: %s\n", Bos.Cmd.to_string(command));
      Bos.OS.Cmd.run_io(~env, ~err, command);
    };
    Ok(exec);
  };
};

let sandboxExec = config => {
  Printf.printf("Sandbox::sandboxExec2: %s\n", Sys.os_type);
  switch (Sys.os_type) {
    | "Win32" => Windows.sandboxExec(config)
    | _ => switch (Run.uname()) {
      | "darwin" => Darwin.sandboxExec(config)
      | _ => NoSandbox.sandboxExec(config)
      };
  };
};
