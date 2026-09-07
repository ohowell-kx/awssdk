# aws-sdk Module

This project is a KDB-X module that wraps the official AWS sdk.

## Features

- Resolves AWS credentials from q using the official AWS authentication chain, so
  environment variables, shared config and credentials files, SSO, and instance or
  container metadata all work without extra configuration.
- Returns credentials as a q dictionary, including the expiry, ready to pass to
  other processes or use in signed requests.
- Configurable AWS SDK log level, set when the session is initialized.

## Installation

aws-sdk is written as a module, under KDB-X's module framework. Though modules can be loaded from anywhere if added to your `$QPATH`, we recommend installing under a `kx` folder within your `$QPATH`. This is to avoid name clashes with other user defined modules, as well as providing a name for other KX modules to cross reference each other.

The module links against the AWS SDK for C++ shared libraries, which must be
present at run time and not only when building. Installing from qmamba resolves
them for you. The release archives contain only the module binary, so if you install
a release you also need the `aws sdk cpp` libraries — see
[Installing dependencies](#installing-dependencies) — and if they are in a
non-standard location, that location must be on your `LD_LIBRARY_PATH`. Loading the
module without them fails with:

```
'no kexport: libaws-cpp-sdk-core.so: cannot open shared object file: No such file or directory
```

### Installing from qmamba

qmamba is a package manager for KDB-X. It is currently available in a private preview capacity. You are welcome to try it and provide feedback.
Follow the install instructions for [qmamba](https://github.com/KxSystems/qmamba/blob/main/README.md#installation).

```q
qmamba:use`kx.qmamba
qmamba.create "myenv"
qmamba.activate "myenv"
qmamba.install `SPECS`CHANNEL!(enlist "q-kx-aws-sdk";("kx";"conda-forge"))
aws:use`kx.awssdk
```

### Installing a release

We recommend you install this module through a release.

[Download a release](https://github.com/KxSystems/awssdk/releases) and then unzip to your module directory. The following example assumes the default install location for KDB-X.

```bash
unzip aws-sdk-l64.zip -d ~/.kx/mod
```

### Installing from source

#### Installing dependencies

Follow the instruction on [AWS documentation](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/sdk-from-source.html) to
install `aws sdk cpp` headers and libraries.

```bash
cd aws-sdk-cpp
export AWS_SDK_PATH=<path/to/install>
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTING=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_ONLY="s3;sts;s3-crt;transfer;iam" -DCMAKE_INSTALL_PREFIX=${AWS_SDK_PATH} ..
cmake --build . --config Release --target install
```

Note: your `$AWS_SDK_PATH` must be available during build time. If you use specific location, you need to add `$AWS_SDK_PATH` to
your `LD_LIBRARY_PATH`

#### Installing aws-sdk

After building it move `awssdk.<platform>.[so|dll]` into your module directory, under `kx`. The following example assumes the default install location for KDB-X.

```bash
git clone https://github.com/KxSystems/awssdk.git
cd awssdk
export INSTALL_PATH=$(pwd)/out/kx
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=$AWS_SDK_PATH -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --target install
cp -r $INSTALL_PATH ~/.kx/mod/
```

## Getting started

Now from anywhere you can import awssdk.

```q
q)aws:use`kx.awssdk
```

Initialize the SDK before calling anything else, and shut it down before leaving the
session:

```q
aws:use `kx.awssdk;
aws.initialize[]
credentials: aws.getCredentials[]
aws.shutDown[]
```

You're ready to check out some of the examples we've provided in [examples](examples/) and the [API reference](#api-reference) to get started

## API reference

These are the API specifications for the aws-sdk module.

### initialize

```q
aws.initialize[options]
```

Initializes the SDK. Must be called before any other function in this module.

| Parameter | Type | Description |
| --- | --- | --- |
| `options` | dictionary or `::` | Optional settings. Pass `::` to accept every default. |

**Options**

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `loglevel` | symbol or string | `INFO` | AWS SDK log level. One of `OFF`, `FATAL`, `ERROR`, `WARN`, `INFO`, `DEBUG`, `TRACE`, matched case-insensitively. |

Keys other than `loglevel` are ignored.

**Returns** `1b` if this call initialized the SDK, `0b` if it was already
initialized. A second call makes no changes, so change the log level by calling
`shutDown` first.

**Errors**

| Error | Cause |
| --- | --- |
| `'type` | `options` is neither a dictionary nor `::`. |
| `'loglevel` | `loglevel` is not one of the values above, or is not a symbol or string. |

```q
q)aws.initialize[]
1b
q)aws.initialize[]
0b
q)aws.shutDown[]
1b
q)aws.initialize[([loglevel:"DEBUG"])]
1b
q)aws.initialize[([loglevel:"asdf"])]
'loglevel
```

The module registers an exit handler when it first initializes, so the SDK is shut
down on process exit even if `shutDown` is never called.

### shutDown

```q
aws.shutDown[]
```

Shuts down the SDK. Call it before exiting the session.

**Returns** `1b` if this call shut down an initialized SDK, `0b` if there was
nothing to shut down.

```q
q)aws.shutDown[]
1b
q)aws.shutDown[]
0b
```

### getCredentials

```q
aws.getCredentials[]
```

Fetches AWS credentials using the official AWS authentication chain.

**Returns** a dictionary of five keys:

| Key | Type | Description |
| --- | --- | --- |
| `accessKey` | string | AWS access key ID. |
| `secretKey` | string | AWS secret access key. |
| `sessionToken` | string | Session token. Empty for long-lived credentials. |
| `accountId` | string | AWS account ID. Empty if the provider does not supply one. |
| `expiration` | long | Expiry as milliseconds since the Unix epoch. Credentials that do not expire report `9223372036854`, the largest value a millisecond timestamp can hold. |

**Errors**

| Error | Cause |
| --- | --- |
| `'uninitialized` | `initialize` has not been called. |

A chain that resolves nothing is not an error. If no credentials can be found, the
call still succeeds and returns the same five keys with every string empty, so check
the result rather than relying on a signal:

```q
q)aws.initialize[]
1b
q)credentials: aws.getCredentials[]
q)key credentials
`accessKey`secretKey`sessionToken`accountId`expiration
q)count credentials`accessKey        / no credentials configured
0
q)1970.01.01D00 + 1000000j * credentials`expiration
2262.04.11D23:47:16.854000000
```

Credentials are not cached, so every call resolves the chain again and returns fresh
credentials. Hold on to the result rather than calling this in a tight loop.

## Conda package

For information on how to build and host a conda package see [`Conda usage`](https://github.com/KxSystems/qmamba/wiki/Build)

## Notice

Copyright (c) 2026 KX Systems, Inc.

Licensed under the Apache License, Version 2.0.
