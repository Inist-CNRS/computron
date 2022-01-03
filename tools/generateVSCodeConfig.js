/* eslint-disable no-template-curly-in-string */

const fs = require('fs');
const fsPromise = fs.promises;
const path = require('path');
const util = require('util');
const exec = util.promisify(require('child_process').exec);

// The only purpose of this function is to fetch the absolute path to the node executable
// and inject it in the VSCode debugger config (.vscode/launch.json). This seems unnecessary
// but I could not find a way to fetch the path from the VSCode environment.
(async function () {
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

  try {
    const { stdout } = await exec('which node');

    launchConfig.configurations[0].program = stdout.trim();
  } catch (err) {
    console.error(err);
    return;
  }

  const vscodePath = path.join(__dirname, '..', '.vscode');
  if (!fs.existsSync(vscodePath)) {
    await fsPromise.mkdir(vscodePath);
  }

  await fsPromise.writeFile(path.join(vscodePath, 'launch.json'), JSON.stringify(launchConfig, null, 2));

  await fsPromise.writeFile(path.join(vscodePath, 'tasks.json'), JSON.stringify(taskConfig, null, 2));
})();
