#!/usr/bin/env node

/* eslint-disable no-template-curly-in-string */

// The only purpose of this script is to fetch the absolute path to the node executable
// and inject it in the VSCode debugger config (.vscode/launch.json). This seems unnecessary
// but I could not find a way to fetch the path from the VSCode environment.

import fs from 'fs/promises';
import { existsSync } from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import util from 'util';
import { exec as execCb } from 'child_process';

const exec = util.promisify(execCb);

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const launchConfig = {
  version: '0.2.0',
  configurations: [
    {
      name: 'Debug',
      type: 'cppdbg',
      request: 'launch',
      preLaunchTask: 'npm: build:debug',
      environment: [
        {
          name: 'DEBUG',
          value: 'true',
        },
      ],
      args: [
        '${workspaceFolder}/test/run.js',
      ],
      cwd: '${workspaceFolder}',
      MIMode: 'gdb',
    },
  ],
};

const taskConfig = {
  version: '2.0.0',
  tasks: [
    {
      type: 'npm',
      script: 'build:debug',
      problemMatcher: [],
      label: 'npm: build:debug',
      detail: 'node-gyp build --debug',
    },
  ],
};

const { stdout } = await exec('which node');

launchConfig.configurations[0].program = stdout.trim();

const vscodePath = path.join(__dirname, '..', '.vscode');
if (!existsSync(vscodePath)) {
  await fs.mkdir(vscodePath);
}

await fs.writeFile(path.join(vscodePath, 'launch.json'), JSON.stringify(launchConfig, null, 2));

await fs.writeFile(path.join(vscodePath, 'tasks.json'), JSON.stringify(taskConfig, null, 2));
