# repo-mine

## Synopsis

Data mining from GitHub organisational repos.

## Setup

Firstly, setup your environment by duplicating the `.env` file:

`cp .env.example .env`

and completing it for your case, where:

  * `DATABASE` is the filename of the SQLite database,
  * `GITHUB_API_TOKEN` is your [personal GitHub access token][token],
  * `GITHUB_ORGANIZATION` is the organisation that you're interested in.

Then, install the dependent libraries:

`pip install -r requirements.txt`

## Usage

## License

This software is released under the terms and conditions of [The MIT
License][license]. Please see the `LICENSE` file for more details.

[license]: http://www.opensource.org/licenses/mit-license.php "The MIT License"
[token]: https://github.com/settings/tokens "Personal GitHub access tokens"
