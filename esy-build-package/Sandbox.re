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

let fileToWrite = "E:/test-file.dat"

let testWrite = (str: string) => {
   let oc = open_out(fileToWrite);
   Printf.fprintf(oc, "%s\n", str);
   let _ = close_out(oc);
};


module Windows = {
    let sandboxExec = config => {
        open Run;
        let exec = (~err, ~env, command) => {
            /* let currBinValue = Astring.String.Map.get("cur__bin", env) */
            /* Out_channel.write_all("test.txt", "Hello world!"); */
            /* let updatedMap = env.update("PATH", (opt) => Bos.OS.Env.opt_var("PATH")); */
            open Bos.OS.Cmd;
            let json = BuildTask.Env.to_yojson(env);
            let jsonString = Yojson.to_string(json);
            testWrite(jsonString);
            /* let commands = Bos.Cmd.to_list(command); */
            /* let normalizedCommands = List.map(normalizePath, commands); */
            let%bind augmentedEsyCommand = EsyBash.toEsyBashCommand(command);

            /* print_endline("[DEBUG]: Running command: " ++ Bos.Cmd.to_string(augmentedEsyCommand)); */
            Bos.OS.Cmd.run(augmentedEsyCommand);

            /* run_io(~err, augmentedEsyCommand) */
        };
        Ok(exec);
    };
}

module NoSandbox = {
  let sandboxExec = _config => {
    let exec = (~err, ~env, command) =>
      Bos.OS.Cmd.run_io(~env, ~err, command);
    Ok(exec);
  };
};

let sandboxExec = config =>
  switch (Run.uname()) {
  | "darwin" => Darwin.sandboxExec(config)
  | _ => NoSandbox.sandboxExec(config)
  };
