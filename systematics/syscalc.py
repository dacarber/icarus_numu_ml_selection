import numpy as np
import pandas as pd
import awkward as ak
import matplotlib.pyplot as plt
import argparse
import uproot
import toml
from tqdm import tqdm
from systools import read_log, calc_multisim_covariance, calc_detector_covariance, calc_statistical_covariance

def main(configuration, caf, channel, output):
    """
    Main function for the calculation of covariance matrices
    corresponding to the configured systematic parameters.

    Parameters
    ----------
    configuration: str
        Path to the systematics configuration file (TOML).
    caf: str
        Path to the CAF file containing the systematic parameter
        weights for the central value sample.
    channel: str
        The name of the signal channel (e.g. 1mu1p).
    output: str
        Base output path for the .npz file containing the covariance
        matrices.

    Returns
    -------
    None
    """

    # Load the configuration file and extract the target reconstructed
    # variables and the header of the log file.
    cfg = toml.load(configuration)
    variables = cfg['general']['variables']
    log = cfg['general']['cv_log']
    header = ['run', 'subrun', 'event', 'nu_id'] + cfg['general']['columns']
    covariances = dict()

    # Unfold the total number of calculations
    # (#calculations) = (#systematic parameters) * (#reconstructed variables)
    systematics = [(k, v, var) for k, v in cfg['sys'].items() for var in variables.keys()]

    # Loop over the configured systematic parameters and reconstructed
    # variables and calculate the corresponding covariance matrices for
    # each combination.
    pbar = tqdm(systematics)
    for sysname, syscfg, sysvar in pbar:
        pbar.set_description(f'{"(systematic, variable) = (" + sysname + ", " + sysvar + ")":^45}')
        syscfg['cv_log'] = log
        syscfg['channel'] = channel
        if syscfg['type'] == 'multisim':
            covariances[f'{sysname}_{sysvar}'] = calc_multisim_covariance(syscfg, caf, header, sysvar, variables[sysvar])
        elif syscfg['type'] == 'detector':
            c, v, r, d = calc_detector_covariance(syscfg, header, sysvar, variables[sysvar])
            covariances[f'{sysname}_{sysvar}'] = d
            covariances[f'{sysname}_{sysvar}_cv'] = c
            covariances[f'{sysname}_{sysvar}_vnominal'] = v
            covariances[f'{sysname}_{sysvar}_rmatrix'] = r
        elif syscfg['type'] == 'stats':
            covariances[f'statistical_{sysvar}'] = calc_statistical_covariance(syscfg, header, sysvar, variables[sysvar])
        else:
            raise ValueError(f'Unknown systematic type: {syscfg["type"]}')
        for g in syscfg['group']:
            gname = f'{g}_{sysvar}'
            covariances[gname] = covariances.get(gname, np.zeros((variables[sysvar][0], variables[sysvar][0]))) + covariances[f'{sysname}_{sysvar}']
        
        # Save the covariance matrices (checkpoint).
        np.savez(f'{output}covariances_{channel}.npz', **covariances)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-w', '--weights')
    parser.add_argument('-c', '--config')
    parser.add_argument('-o', '--output', default='./')
    parser.add_argument('-t', '--channel', default='1mu1p')
    args = parser.parse_args()
    main(args.config, args.weights, args.channel, args.output)