/**
 * @flow
 */

import type {CommandContext} from './esy';

import {getSandbox, getBuildConfig} from './esy';
import * as Task from '../build-task.js';
import * as Sandbox from '../sandbox';
import * as Env from '../environment';

export default async function esySandboxEnv(ctx: CommandContext) {
  const sandbox = await getSandbox(ctx);
  const config = await getBuildConfig(ctx);
  const task = Task.fromSandbox(sandbox, config);
  console.log(Env.printEnvironment(task.env));
}

export const noHeader = true;
