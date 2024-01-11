/**
 * @file variables.h
 * @brief Header file for definitions of selection variables.
 * @author justin.mueller@colostate.edu
*/
#ifndef VARIABLES_H
#define VARIABLES_H

#include <algorithm>

namespace vars
{

    /**
     * Variable for counting interactions/particles.
     * @tparam T the type of object (true or reco, interaction or particle).
     * @param interaction/particle to apply the variable on.
     * @return 1.0 (always).
     */
    template<class T>
        double count(const T & obj) { return 1.0; }

    /**
     * Variable for enumerating interaction categories. This is a basic
     * categorization using only signal, neutrino background, and cosmic
     * background as the three categories.
     * @tparam T the type of interaction (true or reco).
     * @param interaction to apply the variable on.
     * @return the enumerated category of the interaction.
    */
    template<class T>
        double category(const T & interaction)
        {
            double cat(2);
            if(cuts::signal_1mu1p(interaction)) cat = 0;
            else if(cuts::other_nu(interaction)) cat = 1;
            return cat;
        }

    /**
     * Variable for enumerating interaction categories. This classifies the
     * interactions based on the visible final states.
     * 0: 1mu1p, 1: 1mu0h, 2: 1muNp (N>1), 3: 1mu1p1pi, 4: nu_mu CC Other, 5: NC, 6: Cosmic
     * @tparam T the type of interaction (true or reco).
     * @param interaction to apply the variable on.
     * @return the enumerated category of the interaction.
    */
    template<class T>
        double category_topology(const T & interaction)
        {
            uint16_t cat(6);
            if(interaction.is_neutrino)
            {
                uint32_t counts[5] = {};
                for(auto &p : interaction.particles)
                {
                    if(p.is_primary)
                    {
                        double energy(p.csda_ke);
                        if constexpr (std::is_same_v<T, caf::SRInteractionTruthDLPProxy>)
                            energy = p.energy_deposit;
                        if((p.pid < 4 && energy > 25) || (p.pid == 4 && energy > 50))
                            ++counts[p.pid];
                    }
                }
                if(counts[0] == 0 && counts[1] == 0 && counts[2] == 1)
                {
                    if(counts[3] == 0 && counts[4] == 1) cat = 0;
                    else if(counts[3] == 0 && counts[4] == 0) cat = 1;
                    else if(counts[3] == 0 && counts[4] > 1) cat = 2;
                    else if(counts[3] == 1 && counts[4] == 1) cat = 3;
                    else if(interaction.nu_current_type == 0) cat = 4;
                }
                else if(interaction.nu_current_type == 0) cat = 4;
                else if(interaction.nu_current_type == 1) cat = 5;
            }
            return cat;
        }

    /**
     * Variable for counting particles in interactions.
     * @tparam T the type of interaction (true or reco).
     * @param interaction to apply the variable on.
     * @return the number of particles in the interaction.
     */
    template<class T>
        double count_particles(const T & interaction) { return interaction.num_particles; }

    /**
     * Variable for counting primaries in interactions.
     * @tparam T the type of interaction (true or reco).
     * @param interaction to apply the variable on.
     * @return the number of primaries in the interaction.
     */
    template<class T>
        double count_primaries(const T & interaction) { return interaction.num_primaries; }
    
    /**
     * Variable for total visible energy of interaction.
     * @tparam T the type of interaction (true or reco).
     * @param interaction to apply the variable on.
     * @return the total visible energy of the interaction.
    */
    template<class T>
        double visible_energy(const T & interaction)
        {
            double energy(0);
            for(const auto & p : interaction.particles)
            {
                if(p.is_primary)
                    energy += p.csda_ke;
                if(p.pid == 2) energy += 105.658;
            }
            return energy;
        }
    
    /**
     * Variable for matched interaction flash time.
     * @tparam T the type of interaction (true or reco).
     * @param interaction to apply the variable on.
     * @return the matched flash time of the interaction.
    */
    template<class T>
        double flash_time(const T & interaction) { return interaction.flash_time; }

    /**
     * Variable for particle primary categorizations.
     * @tparam T the type of particle (true or reco).
     * @param particle to apply the variable on.
     * @return the primary/non-primary designation of the particle.
    */
    template<class T>
        double primary(const T & particle) { return particle.is_primary ? 1 : 0; }

    /**
     * Variable for particle PID.
     * @tparam T the type of particle (true or reco).
     * @param particle to apply the variable on.
     * @return the PID of the particle.
    */
    template<class T>
        double pid(const T & particle) { return particle.pid; }

    /**
     * Variable for particle PID + primary.
     * @tparam T the type of particle (true or reco).
     * @param particle to apply the variable on.
     * @return the PID+primary information for the particle.
    */
    template<class T>
        double primary_pid(const T & particle) { return particle.pid + (particle.is_primary ? 5 : 0); }

    /**
     * Variable for particle csda_ke.
     * @tparam T the type of particle (true or reco).
     * @param particle to apply the variable on.
     * @return the csda_ke of the particle.
    */
    template<class T>
        double csda_ke(const T & particle) { return particle.csda_ke; }

    /**
     * Variable for the particles lowest x-coordinate.
     * @tparam T the type of particle (true or reco)
     * @param particle to apply the variable on.
     * @return the lowest x-coordinate of the particle start/end points.
    */
    template<class T>
        double lowx(const T & particle) { return std::min(particle.start_point[0], particle.end_point[0]); }
}

#endif