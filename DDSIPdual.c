/*  Authors:           Andreas M"arkert, Ralf Gollmer
	Copyright to:      University of Duisburg-Essen
    Language:          C
    Last modification: 16.04.2016
	Description:
	This file contains the implementation of the dual method, i.e. all
	functions required by ConicBundle of C. Helmberg.

	License:
	This file is part of DDSIP.

    DDSIP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    DDSIP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DDSIP; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

//#define MDEBUG

#ifdef CONIC_BUNDLE
// compile only in case ConicBundle ist available

#include <DDSIP.h>

#include <DDSIPconst.h>


//==========================================================================
int
DDSIP_NonAnt (void)
{
    int status = 0, scen, i, k, j;

    DDSIP_data->nabeg = (int *) DDSIP_Alloc (sizeof (int), DDSIP_bb->firstvar * DDSIP_param->scenarios, "nabeg(NonAnt)");
    DDSIP_data->nacnt = (int *) DDSIP_Alloc (sizeof (int), DDSIP_bb->firstvar * DDSIP_param->scenarios, "nacnt(NonAnt)");

    if (DDSIP_param->nonant == 3)
    {
        DDSIP_data->naind =
            (int *) DDSIP_Alloc (sizeof (int), DDSIP_bb->firstvar * DDSIP_param->scenarios * (DDSIP_param->scenarios - 1), "naind(NonAnt)");
        DDSIP_data->naval =
            (double *) DDSIP_Alloc (sizeof (double), DDSIP_bb->firstvar * DDSIP_param->scenarios * (DDSIP_param->scenarios - 1), "naval(NonAnt)");
    }
    else
    {
        DDSIP_data->naind = (int *) DDSIP_Alloc (sizeof (int), 2 * DDSIP_bb->firstvar * DDSIP_param->scenarios, "naind(NonAnt)");
        DDSIP_data->naval = (double *) DDSIP_Alloc (sizeof (double), 2 * DDSIP_bb->firstvar * DDSIP_param->scenarios, "naval(NonAnt)");
    }

    // x1=x2, x2=x3 ... x(S-1)=xS
    if (DDSIP_param->nonant == 2)
    {
        k = 0;
        for (scen = 0; scen < DDSIP_param->scenarios; scen++)
            for (i = 0; i < DDSIP_bb->firstvar; i++)
            {
                DDSIP_data->nabeg[scen * DDSIP_bb->firstvar + i] = k;
                if (scen && scen < DDSIP_param->scenarios - 1)
                    DDSIP_data->nacnt[scen * DDSIP_bb->firstvar + i] = 2;
                else
                    DDSIP_data->nacnt[scen * DDSIP_bb->firstvar + i] = 1;

                if (scen)
                {
                    DDSIP_data->naind[k] = (scen - 1) * DDSIP_bb->firstvar + i;
                    DDSIP_data->naval[k++] = -1.0;
                }
                if (scen < DDSIP_param->scenarios - 1)
                {
                    DDSIP_data->naind[k] = scen * DDSIP_bb->firstvar + i;
                    DDSIP_data->naval[k++] = 1.0;
                }
            }
    }
    // x_j = sum p_i x_i
    else if (DDSIP_param->nonant == 3)
    {
        k = 0;
        for (scen = 0; scen < DDSIP_param->scenarios; scen++)
            for (i = 0; i < DDSIP_bb->firstvar; i++)
            {
                DDSIP_data->nabeg[scen * DDSIP_bb->firstvar + i] = k;
                DDSIP_data->nacnt[scen * DDSIP_bb->firstvar + i] = DDSIP_param->scenarios - 1;

                for (j = 0; j < DDSIP_param->scenarios-1; j++)
                {
                    DDSIP_data->naind[k] = j * DDSIP_bb->firstvar + i;
                    if (j == scen)
                        DDSIP_data->naval[k++] = 1 - DDSIP_data->prob[scen];
                    else
                        DDSIP_data->naval[k++] = -DDSIP_data->prob[scen];
                }
            }
    }
    // x1=x2, x1=x3 ... x1=xS
    else
    {
        k = 0;
        // First scenario
        for (i = 0; i < DDSIP_bb->firstvar; i++)
        {
            DDSIP_data->nabeg[i] = k;
            DDSIP_data->nacnt[i] = DDSIP_param->scenarios - 1;

            for (scen = 0; scen < DDSIP_param->scenarios - 1; scen++)
            {
                DDSIP_data->naind[k] = scen * DDSIP_bb->firstvar + i;
                DDSIP_data->naval[k++] = 1.0;
            }
        }
        // Other scenarios
        for (i = 0; i < DDSIP_bb->firstvar; i++)
            for (scen = 1; scen < DDSIP_param->scenarios; scen++)
            {
                DDSIP_data->nabeg[scen * DDSIP_bb->firstvar + i] = k;
                DDSIP_data->nacnt[scen * DDSIP_bb->firstvar + i] = 1;
                DDSIP_data->naind[k] = (scen - 1) * DDSIP_bb->firstvar + i;
                DDSIP_data->naval[k++] = -1.0;
            }
    }

    if (DDSIP_param->outlev > 97)
    {
        int rows = 0, r, surplus;
        fprintf (DDSIP_bb->moreoutfile, "\n----------------------\n");
//    fprintf (DDSIP_bb->moreoutfile, "Nonanticipativity matrix H:\n");
//    fprintf (DDSIP_bb->moreoutfile, "Col  Row  Val\n");
        for (i = 0; i < DDSIP_bb->firstvar * DDSIP_param->scenarios; i++)
            for (k = DDSIP_data->nabeg[i]; k < DDSIP_data->nabeg[i] + DDSIP_data->nacnt[i]; k++)
            {
//  	fprintf (DDSIP_bb->moreoutfile, "%-4d  %-4d  %-16.12g\n", i + 1, DDSIP_data->naind[k] + 1, DDSIP_data->naval[k]);
                if (DDSIP_data->naind[k] > rows)
                    rows = DDSIP_data->naind[k];
            }
//    fprintf (DDSIP_bb->moreoutfile, "\n");
        fprintf (DDSIP_bb->moreoutfile, "\nNonanticipativity constraints:\n");
        fprintf (DDSIP_bb->moreoutfile, "( %d rows )\n",rows+1);
        for (r=0; r<=rows; r++)
        {
            for (i = 0; i < DDSIP_bb->firstvar; i++)
            {
                if (CPXgetcolname(DDSIP_env, DDSIP_lp, DDSIP_bb->name_buffer, DDSIP_bb->n_buffer, DDSIP_bb->n_buffer_len, &surplus, DDSIP_bb->firstindex[i], DDSIP_bb->firstindex[i]))
                {
                    fprintf (stderr,"Error in querying columns name for index %d\n",DDSIP_bb->firstindex[i]);
                    exit (1);
                }
                for (scen = 0; scen < DDSIP_param->scenarios; scen++)
                {
                    for (j = DDSIP_data->nabeg[scen * DDSIP_bb->firstvar + i];
                            j < DDSIP_data->nabeg[scen * DDSIP_bb->firstvar + i] + DDSIP_data->nacnt[scen * DDSIP_bb->firstvar + i]; j++)
                    {
                        if (DDSIP_data->naind[j] == r)
                        {
                            if (DDSIP_data->naval[j] >= 0.)
                                fprintf (DDSIP_bb->moreoutfile, " +%g * %s_SC%.2d",DDSIP_data->naval[j],DDSIP_bb->n_buffer,scen);
                            else
                                fprintf (DDSIP_bb->moreoutfile, " %g * %s_SC%.2d",DDSIP_data->naval[j],DDSIP_bb->n_buffer,scen);
                        }
                    }
                }
            }
            fprintf (DDSIP_bb->moreoutfile, " = 0\n");
        }
    }
    return status;
}

//==========================================================================
// This function is called by Helmbergs conic bundle method to update the subgradient
// and the objective value
// Note: We maximize with respect to the Lagrangian multipliers, Helmberg minimizes
int
DDSIP_DualUpdate (void *function_key, double *dual, double relprec,
                  int max_new_subg, double *objective_value, int *new_subg,
                  double *subgval, double *subgradient, double *primal)
{
    int status, scen, i, j;


//  if (DDSIP_param->outlev > 50) {
//      printf("----- Call of DualUpdate with relprec= %g\n", relprec);
//      fprintf(DDSIP_bb->moreoutfile, "---- Call of DualUpdate with relprec= %g\n", relprec);
//  }
//  if (DDSIP_param->outlev > 50) {
//    printf(" Call of DualUpdate for new subgradient in dual (dimension %d):\n",DDSIP_bb->dimdual);
//    for (i=0; i<DDSIP_bb->dimdual; i++){
//      printf (" %3d: %g,",i,dual[i]);
//      if (!((i+1)%10))
//        printf ("\n");
//    }
//    printf("__________\n");
//  }

    if (DDSIP_killsignal)
    {
        fprintf (DDSIP_outfile, "\nTermination signal received.\n");
        if (DDSIP_param->outlev)
            fprintf (DDSIP_bb->moreoutfile, "\nTermination signal received.\n");
        *new_subg = 0;
        return -1;
    }

    // update node->dual
    memcpy (DDSIP_node[DDSIP_bb->curnode]->dual, dual, sizeof (double) * DDSIP_bb->dimdual);

    if (DDSIP_param->outlev > 50)
    {
        fprintf (DDSIP_bb->moreoutfile, "\nCurrent lambda for node %d: (%p)\n", DDSIP_bb->curnode, DDSIP_node[DDSIP_bb->curnode]->dual);
        for (i = 0; i < DDSIP_bb->dimdual; i++)
        {
            fprintf (DDSIP_bb->moreoutfile, " %14.8g,", DDSIP_node[DDSIP_bb->curnode]->dual[i]);
            if (!((i+1)%10))
                fprintf (DDSIP_bb->moreoutfile, "\n");
        }
        fprintf(DDSIP_bb->moreoutfile, "\n");
    }
    // LowerBound scenario problems
    if ((status = DDSIP_CBLowerBound (objective_value, relprec)) > 1)
    {
        printf ("Failed to solve scenario problems: status = %d .\n", status);
        *new_subg = 0;
        return status;
    }

    DDSIP_bb->dualitcnt++;

    // Calculate new subgradient H * x
    // Initialization
    for (scen = 0; scen < DDSIP_param->scenarios - 1; scen++)
        for (i = 0; i < DDSIP_bb->firstvar; i++)
            subgradient[scen * DDSIP_bb->firstvar + i] = 0.0;

    if (DDSIP_param->scalarization)
    {
        if (DDSIP_param->outlev > 29)
        {
            printf ("DDSIP_bb->ref_max= %d, DDSIP_bb->risk:", DDSIP_bb->ref_max);
            for (scen = 0; scen < DDSIP_param->scenarios; scen++)
                printf (" %g|", DDSIP_bb->ref_risk[scen]);
        }
        for (i = 0; i < DDSIP_bb->firstvar; i++)
        {
            for (scen = 0; scen < DDSIP_param->scenarios; scen++)
            {
                for (j = DDSIP_data->nabeg[scen * DDSIP_bb->firstvar + i];
                        j < DDSIP_data->nabeg[scen * DDSIP_bb->firstvar + i] + DDSIP_data->nacnt[scen * DDSIP_bb->firstvar + i]; j++)
                {
                    subgradient[DDSIP_data->naind[j]] -= DDSIP_data->naval[j] * (DDSIP_node[DDSIP_bb->curnode]->first_sol)[scen][i] / DDSIP_param->ref_scale[0];
                    if (DDSIP_param->outlev > 29)
                        printf (" Subgradient[%d]: Scenario %d, Variable %d (Werte: %g): -= %g\n", DDSIP_data->naind[j], scen, i,
                                (DDSIP_node[DDSIP_bb->curnode]->first_sol)[scen][i],
                                DDSIP_data->naval[j] * (DDSIP_node[DDSIP_bb->curnode]->first_sol)[scen][i] / DDSIP_param->ref_scale[0]);
                }
            }
        }
        if (DDSIP_param->outlev > 29)
            printf ("\n");
        if (DDSIP_param->outlev > 29)
            printf ("   curexp=%g, currisk=%g\n", DDSIP_bb->curexp, DDSIP_bb->currisk);
    }
    else
    {
        for (i = 0; i < DDSIP_bb->firstvar; i++)
            for (scen = 0; scen < DDSIP_param->scenarios; scen++)
                for (j = DDSIP_data->nabeg[scen * DDSIP_bb->firstvar + i];
                        j < DDSIP_data->nabeg[scen * DDSIP_bb->firstvar + i] + DDSIP_data->nacnt[scen * DDSIP_bb->firstvar + i]; j++)
                {
                    if (DDSIP_killsignal)
                    {
                        fprintf (DDSIP_outfile, "\nTermination signal received.\n");
                        if (DDSIP_param->outlev)
                            fprintf (DDSIP_bb->moreoutfile, "\nTermination signal received.\n");
                        *new_subg = 0;
                        return -1;
                    }
                    subgradient[DDSIP_data->naind[j]] -= DDSIP_data->naval[j] * (DDSIP_node[DDSIP_bb->curnode]->first_sol)[scen][i];
                }
    }
    //
    subgval[0] = *objective_value;
    //DDSIP_bb->dualObjVal =  -(*objective_value);
    if (DDSIP_param->outlev > 29)
    {
        fprintf (DDSIP_bb->moreoutfile, "\n After scenarios solving: (Total it. %d in node %d) ", DDSIP_bb->dualitcnt, DDSIP_bb->curnode);
        fprintf (DDSIP_bb->moreoutfile, "\n SC  VAR     FIRSTSOL         LAMBDA       returned subgradient (=H*FIRSTSOL)\n");
        printf ("\n After scenarios solving: (Total it. %d in node %d) ", DDSIP_bb->dualitcnt, DDSIP_bb->curnode);
        printf ("\n SC  VAR     FIRSTSOL         LAMBDA       returned subgradient (=H*FIRSTSOL)\n");
        for (scen = 0; scen < DDSIP_param->scenarios - 1; scen++)
            for (i = 0; i < DDSIP_bb->firstvar; i++)
            {
                fprintf (DDSIP_bb->moreoutfile,
                         "%3d  %3d  %15.10f   %15.10f     %22.14g\n", scen + 1,
                         i + 1, (DDSIP_node[DDSIP_bb->curnode]->first_sol)[scen][i],
                         dual[scen * DDSIP_bb->firstvar + i], subgradient[scen * DDSIP_bb->firstvar + i]);
                printf (
                    "%3d  %3d  %15.10f   %15.10f     %22.14g\n", scen + 1,
                    i + 1, (DDSIP_node[DDSIP_bb->curnode]->first_sol)[scen][i],
                    dual[scen * DDSIP_bb->firstvar + i], subgradient[scen * DDSIP_bb->firstvar + i]);
            }
        for (i = 0; i < DDSIP_bb->firstvar; i++)
        {
            fprintf (DDSIP_bb->moreoutfile, "%3d  %3d  %15.10f\n",
                     DDSIP_param->scenarios, i + 1, (DDSIP_node[DDSIP_bb->curnode]->first_sol)[DDSIP_param->scenarios - 1][i]);
            printf ("%3d  %3d  %15.10f\n",
                    DDSIP_param->scenarios, i + 1, (DDSIP_node[DDSIP_bb->curnode]->first_sol)[DDSIP_param->scenarios - 1][i]);
        }
        fprintf (DDSIP_bb->moreoutfile," Dual objective    = %18.12f\n", *objective_value);
        fprintf (DDSIP_bb->moreoutfile,"-DDSIP_node->bound = %18.12f\n", -DDSIP_node[DDSIP_bb->curnode]->bound);
        printf (" Dual objective    = %18.12f\n", *objective_value);
        printf ("-DDSIP_node->bound = %18.12f\n", -DDSIP_node[DDSIP_bb->curnode]->bound);
        fprintf (DDSIP_bb->moreoutfile, "\n");
        printf ("\n");
    }
    *new_subg = 1;
    if (DDSIP_killsignal)
    {
        fprintf (DDSIP_outfile, "\nTermination signal received.\n");
        if (DDSIP_param->outlev)
            fprintf (DDSIP_bb->moreoutfile, "\nTermination signal received.\n");
        *new_subg = 0;
        return -1;
    }
    return 0;
}

//==========================================================================
// Main procedure of Helmbergs conic bundle implementation
int
DDSIP_DualOpt (void)
{
    cb_problemp p;
    int cb_status, status, i_scen, j, cnt, noIncreaseCounter = 0;
    double *minfirst = (double *) DDSIP_Alloc (sizeof (double), DDSIP_bb->firstvar,
                       "minfirst(DualOpt)");
    double *maxfirst = (double *) DDSIP_Alloc (sizeof (double), DDSIP_bb->firstvar,
                       "maxfirst(DualOpt)");
    double *center_point = (double *) DDSIP_Alloc (sizeof (double), DDSIP_bb->dimdual,
                           "center_point(DualOpt)");
    double obj, old_obj, start_weight;
    int    cpu_hrs, cpu_mins, limits_reset, last_dualitcnt = 1, repeated_increase = 1, weight_decreases = 0, many_iters = 0;
    double cpu_secs;
    double old_cpxrelgap = 1.e-16, old_cpxtimelim = 1000000., old_cpxrelgap2 = 1.e-16, old_cpxtimelim2 = 1000000., last_weight, next_weight, reduction_factor = 0.5;

    if (DDSIP_param->outlev)
    {
        fprintf (DDSIP_bb->moreoutfile, "\n----------------------\n");
        fprintf (DDSIP_bb->moreoutfile, "Invoking ConicBundle...\n");
    }
    // New cplex parameters
    if (DDSIP_param->cpxnodual)
    {
        status = DDSIP_SetCpxPara (DDSIP_param->cpxnodual, DDSIP_param->cpxdualisdbl, DDSIP_param->cpxdualwhich, DDSIP_param->cpxdualwhat);
        if (status)
        {
            fprintf (stderr, "ERROR: Failed to set CPLEX parameters (Dual) \n");
            DDSIP_Free ((void **) &(minfirst));
            DDSIP_Free ((void **) &(maxfirst));
            DDSIP_Free ((void **) &(center_point));
            return status;
        }
    }

    p = cb_construct_problem (0);
    if (p == 0)
    {
        printf ("construct_problem failed\n");
        DDSIP_Free ((void **) &(minfirst));
        DDSIP_Free ((void **) &(maxfirst));
        DDSIP_Free ((void **) &(center_point));
        return 1;
    }
    DDSIP_bb->dualProblem = p;
    cb_clear (p);
    cb_set_defaults (p);
    if (cb_init_problem (p, DDSIP_bb->dimdual, NULL, NULL))
    {
        printf ("init_problem failed\n");
        cb_destruct_problem (&p);
        DDSIP_Free ((void **) &(minfirst));
        DDSIP_Free ((void **) &(maxfirst));
        DDSIP_Free ((void **) &(center_point));
        return 1;
    }
    if (cb_add_function (p, (void *) DDSIP_DualUpdate, DDSIP_DualUpdate, 0, 0))
    {
        printf ("add DUAL_UPDATE failed\n");
        cb_destruct_problem (&p);
        DDSIP_Free ((void **) &(minfirst));
        DDSIP_Free ((void **) &(maxfirst));
        DDSIP_Free ((void **) &(center_point));
        return 1;
    }
    if (cb_reinit_function_model(p, (void *) DDSIP_DualUpdate))
    {
        printf ("reinit_function_model failed\n");
        cb_destruct_problem (&p);
        DDSIP_Free ((void **) &(minfirst));
        DDSIP_Free ((void **) &(maxfirst));
        DDSIP_Free ((void **) &(center_point));
        return 1;
    }
    DDSIP_bb->dualObjVal = -DDSIP_infty;

    cb_set_print_level (p, DDSIP_param->cbprint);
    cb_set_term_relprec (p, DDSIP_param->cbrelgap);
    cb_set_eval_limit (p, DDSIP_param->cbtotalitlim);
    cb_set_max_bundlesize (p, (void *) DDSIP_DualUpdate, DDSIP_param->cbbundlesz);
    cb_set_max_new_subgradients (p, (void *) DDSIP_DualUpdate, DDSIP_param->cbmaxsubg);

    // The choice of the starting weight influences the convergence and results of Conic Bundle.
    // Is there a generally good choice?
    // More testing has to be done!
    if (DDSIP_bb->curnode)
    {
        start_weight = DDSIP_param->cbfactor*DDSIP_param->cbweight + (1. - DDSIP_param->cbfactor)*DDSIP_node[DDSIP_bb->curnode]->dual[DDSIP_bb->dimdual];
        // idea: limit too big start weights - speed up or slow down?
#define BigWeight 400.
        if (start_weight > BigWeight && (!DDSIP_param->riskmod ||start_weight > DDSIP_param->riskweight))
            start_weight = (start_weight-BigWeight)*0.5  + BigWeight;
        if (DDSIP_param->riskmod == 4)
        {
            if (start_weight < DDSIP_param->riskweight*0.2)
                start_weight = DDSIP_param->riskweight*0.2;
        }
#ifdef MDEBUG
////////////////////////////////////////////
        if(DDSIP_param->outlev)
            fprintf (DDSIP_bb->moreoutfile," ******************** node %d, father %d inherited dual %p with weight %g, mean start_weight: %g\n", DDSIP_bb->curnode, DDSIP_node[DDSIP_bb->curnode]->father, DDSIP_node[DDSIP_bb->curnode]->dual, DDSIP_node[DDSIP_bb->curnode]->dual[DDSIP_bb->dimdual],start_weight);
////////////////////////////////////////////
#endif
    }
    else
        start_weight = DDSIP_param->cbweight;
    last_weight = next_weight = start_weight;
    if (start_weight >= 0.)
    {
        cb_set_next_weight (p, start_weight);
#ifdef MDEBUG
////////////////////////////////////////////
        if(DDSIP_param->outlev)
            fprintf (DDSIP_bb->moreoutfile, " setting next weight for node %d: %g\n", DDSIP_bb->curnode, start_weight);
////////////////////////////////////////////
#endif
    }
    else
    {
        last_weight = next_weight = 11.12345;
        cb_set_next_weight (p, 11.12345);
#ifdef MDEBUG
////////////////////////////////////////////
        if(DDSIP_param->outlev)
            fprintf (DDSIP_bb->moreoutfile, " start_weight computed to be %g, setting next weight: %g\n", start_weight, 11.2345);
////////////////////////////////////////////
#endif
    }
    DDSIP_bb->dualitcnt    = 0;
    DDSIP_bb->dualdescitcnt = 0;
    if (DDSIP_param->outlev)
        fprintf (DDSIP_bb->moreoutfile, "\nInitial dual evaluation");

    // paranoid dimension check
    if (cb_get_dim(p) != DDSIP_bb->dimdual)
    {
        printf ("XXX Error in dimension of the conic bundle problem: is %d, should be %d.\n",cb_get_dim(p),DDSIP_bb->dimdual);
        exit (1);
    }
    // Initialize multipliers (0 in root node, multipliers of father node otherwise)
    if ((status = cb_set_new_center_point (p, DDSIP_node[DDSIP_bb->curnode]->dual)))
    {
        printf ("set_new_center_point returned %d\n", status);
        cb_destruct_problem (&p);
        DDSIP_Free ((void **) &(minfirst));
        DDSIP_Free ((void **) &(maxfirst));
        DDSIP_Free ((void **) &(center_point));
        return status;
    }

    // use the result of set_center as initial dualObjVal
    old_obj = obj = DDSIP_bb->dualObjVal;
    limits_reset = 0;
    if (DDSIP_param->cb_changetol)
    {
        if (obj < DDSIP_node[DDSIP_bb->curnode]->bound - 1.e-5)
        {
            // the branched problems produced a worse bound than in the father node - reset temporarily cplex tolerance and time limit
            limits_reset = 1;
            printf("Initial evaluation at branched node gave obj=%g, worse than bound %g from father node - reset CPLEX relgap and/or time limit.\n", obj, DDSIP_node[DDSIP_bb->curnode]->bound);
            if (DDSIP_param->outlev)
                fprintf(DDSIP_bb->moreoutfile, "Initial evaluation at branched node gave obj=%g, worse than bound %g from father node - reset CPLEX relgap and/or time limit.\n", obj, DDSIP_node[DDSIP_bb->curnode]->bound);
            for (cpu_hrs=0; cpu_hrs < DDSIP_param->cpxnodual; cpu_hrs++)
            {
                if (DDSIP_param->cpxdualwhich[cpu_hrs] == CPX_PARAM_TILIM)
                {
                    old_cpxtimelim = DDSIP_param->cpxdualwhat[cpu_hrs];
                    printf(" reset: old_timelim= %g, DDSIP_param->cpxdualwhich[%d]= %d, what= %g\n", old_cpxtimelim, cpu_hrs, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                    DDSIP_param->cpxdualwhat[cpu_hrs] = 1.5*old_cpxtimelim;
                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                    if (status)
                    {
                        printf("Failed to reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                    }
                    else
                    {
                        printf("temporarily reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                        if (DDSIP_param->outlev)
                            fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                    }
                }
                if (DDSIP_param->cpxdualwhich[cpu_hrs] == CPX_PARAM_EPGAP)
                {
                    old_cpxrelgap = DDSIP_param->cpxdualwhat[cpu_hrs];
                    printf(" reset: old_relgap = %g, DDSIP_param->cpxdualwhich[%d]= %d, what= %g\n", old_cpxtimelim, cpu_hrs, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                    DDSIP_param->cpxdualwhat[cpu_hrs] = 0.10*old_cpxrelgap;
                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                    if (status)
                    {
                        printf("Failed to reset CPLEX relgap to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                    }
                    else
                    {
                        printf("temporarily reset CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                        if (DDSIP_param->outlev)
                            fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                    }
                }
            }
            for (cpu_hrs=0; cpu_hrs < DDSIP_param->cpxnodual2; cpu_hrs++)
            {
                if (DDSIP_param->cpxdualwhich2[cpu_hrs] == CPX_PARAM_TILIM)
                {
                    old_cpxtimelim2 = DDSIP_param->cpxdualwhat2[cpu_hrs];
//printf(" reset: old_timelim2= %g, DDSIP_param->cpxdualwhich2[%d]= %d, what2= %g\n", old_cpxtimelim2, cpu_hrs, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    DDSIP_param->cpxdualwhat2[cpu_hrs] = 1.5*old_cpxtimelim2;
                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    if (status)
                    {
                        printf("Failed to reset CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    }
                    else
                    {
                        printf("temporarily reset CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                        if (DDSIP_param->outlev)
                            fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    }
                }
                if (DDSIP_param->cpxdualwhich2[cpu_hrs] == CPX_PARAM_EPGAP)
                {
                    old_cpxrelgap2 = DDSIP_param->cpxdualwhat2[cpu_hrs];
//printf(" reset: old_relgap2 = %g, DDSIP_param->cpxdualwhich2[%d]= %d, what2= %g\n", old_cpxtimelim2, cpu_hrs, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    DDSIP_param->cpxdualwhat2[cpu_hrs] = 0.10*old_cpxrelgap2;
                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    if (status)
                    {
                        printf("Failed to reset CPLEX relgap 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    }
                    else
                    {
                        printf("temporarily reset CPLEX relgap tolerance 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                        if (DDSIP_param->outlev)
                            fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX relgap tolerance 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    }
                }
            }
            // reevaluate initial point with changed parameters
            if ((status = cb_set_new_center_point (p, DDSIP_node[DDSIP_bb->curnode]->bestdual)))
            {
                printf ("set_new_center_point returned %d\n", status);
                cb_destruct_problem (&p);
                DDSIP_Free ((void **) &(minfirst));
                DDSIP_Free ((void **) &(maxfirst));
                DDSIP_Free ((void **) &(center_point));
                return status;
            }
            if (DDSIP_param->outlev)
                fprintf (DDSIP_bb->moreoutfile, "set_new_center_point to bestdual, reevaluate with new tolerances and limits\n");
            old_obj = obj = DDSIP_bb->dualObjVal;
        }
    }

    if (!DDSIP_bb->curnode)
    {
        int comb, i;
        double tmpbestheur;

        printf ("\nUpper bounds for initial solution\n");
        if (DDSIP_param->outlev)
            fprintf (DDSIP_bb->moreoutfile, "\nUpper bounds for initial solution\n");
        DDSIP_bb->DDSIP_step = neobj;
        // Initialize, heurval contains the current heuristic solution
        DDSIP_bb->heurval = DDSIP_infty;
        tmpbestheur = DDSIP_infty;
        // sort scenarios according to lower bounds for the initial solution
        {
            // in order to allow for premature cutoff: sort scenarios according to lower bound in root node in descending order
            double * sort_array;
            sort_array = (double *) DDSIP_Alloc(sizeof(double), DDSIP_param->scenarios, "sort_array(LowerBound)");
            for (i_scen = 0; i_scen < DDSIP_param->scenarios; i_scen++)
            {
                sort_array[i_scen] = DDSIP_data->prob[i_scen] * (DDSIP_node[DDSIP_bb->curnode]->subbound)[i_scen];
            }
            DDSIP_qsort_ins_D (sort_array, DDSIP_bb->scen_order, 0, DDSIP_param->scenarios-1);
            DDSIP_Free ((void**) &sort_array);

        }
        if (DDSIP_param->heuristic == 100)  	// use subsequently different heuristics in the same node
        {
            for (i = 1; i < DDSIP_param->heuristic_num; i++)
            {
                DDSIP_param->heuristic = floor (DDSIP_param->heuristic_vector[i] + 0.1);
                if (!DDSIP_Heuristics (&comb))
                {
                    // Evaluate the proposed first-stage solution (if DDSIP_bb->skip was not set)
                    if (DDSIP_bb->sug[DDSIP_param->nodelim + 2])
                    {
                        if (!DDSIP_UpperBound ())
                        {
                            if (DDSIP_bb->heurval < tmpbestheur)
                                tmpbestheur = DDSIP_bb->heurval;
                        }
                    }
                }
            }
            DDSIP_param->heuristic = 12;
            if (!DDSIP_Heuristics (&comb))
            {
                // Evaluate the proposed first-stage solution (if DDSIP_bb->skip was not set)
                if (DDSIP_bb->sug[DDSIP_param->nodelim + 2])
                {
                    if (!DDSIP_UpperBound ())
                    {
                        if (DDSIP_bb->heurval < tmpbestheur)
                            tmpbestheur = DDSIP_bb->heurval;
                    }
                }
            }
            DDSIP_param->heuristic = 100;
            DDSIP_bb->heurval = tmpbestheur;
        }
        else if (DDSIP_param->heuristic == 99)  	// use subsequently different heuristics in the same node
        {
            for (i = 1; i < DDSIP_param->heuristic_num; i++)
            {
                DDSIP_param->heuristic = floor (DDSIP_param->heuristic_vector[i] + 0.1);
                if (!DDSIP_Heuristics (&comb))
                {
                    // Evaluate the proposed first-stage solution
                    if (!DDSIP_UpperBound ())
                    {
                        if (DDSIP_bb->heurval < tmpbestheur)
                            tmpbestheur = DDSIP_bb->heurval;
                    }
                }
            }
            DDSIP_param->heuristic = 99;
            DDSIP_bb->heurval = tmpbestheur;
        }
        else
        {
            if (!DDSIP_Heuristics (&comb))
                // Evaluate the proposed first-stage solution
                DDSIP_UpperBound ();
        }
        DDSIP_bb->DDSIP_step = dual;
    }

    if (DDSIP_param->outlev)
    {
        DDSIP_translate_time (DDSIP_GetCpuTime(),&cpu_hrs,&cpu_mins,&cpu_secs);
        fprintf (DDSIP_outfile, "\n   --------- Dual:  Descent    Total  Objective        Weight        Bound                              CPU Time");
        printf ("\n   --------- Dual:  Descent    Total  Objective        Weight        Bound                              CPU Time");
        fprintf (DDSIP_outfile,"\n  |                       0        1  %-16.12g %-11.6g   %-16.12g%20dh %02d:%05.2f\n", obj, start_weight, DDSIP_node[DDSIP_bb->curnode]->bound, cpu_hrs,cpu_mins,cpu_secs);
        printf ("\n  |                       0        1  %-16.12g %-11.6g   %-16.12g%20dh %02d:%05.2f\n", obj, start_weight, DDSIP_node[DDSIP_bb->curnode]->bound, cpu_hrs,cpu_mins,cpu_secs);
    }

    if (DDSIP_node[DDSIP_bb->curnode]->bound > DDSIP_bb->bestvalue - fabs(DDSIP_bb->bestvalue)*1.e-15)
    {
        if (DDSIP_param->outlev)
        {
            if ((DDSIP_node[DDSIP_bb->curnode]->bound - DDSIP_bb->bestvalue)/(fabs(DDSIP_bb->bestvalue)+1.e-10) > DDSIP_param->accuracy)
                DDSIP_Print2 ("   --------- termination status: cutoff.", "\n", 0, 0);
            else
                DDSIP_Print2 ("   --------- termination status: optimal.", "\n", 0, 0);
        }
    }
    else
    {
        noIncreaseCounter = 0;
        many_iters = 0;
        while (!cb_termination_code (p) && (DDSIP_GetCpuTime () < DDSIP_param->timelim)
                && ((DDSIP_bb->curnode && DDSIP_bb->dualdescitcnt < DDSIP_param->cbitlim && (DDSIP_bb->curnode >= DDSIP_param->cbBreakIters || DDSIP_bb->dualdescitcnt < (DDSIP_param->cbitlim+1)/2))
                    || (!DDSIP_bb->curnode && DDSIP_bb->dualdescitcnt < DDSIP_param->cbrootitlim))
                && DDSIP_bb->dualitcnt < DDSIP_param->cbtotalitlim && !(obj > DDSIP_bb->bestvalue - DDSIP_param->accuracy) && noIncreaseCounter < 9)
        {
            DDSIP_bb->dualdescitcnt++;
            if (DDSIP_bb->dualdescitcnt <= 1)
                DDSIP_bb->dualObjVal = -DDSIP_infty;

            if (DDSIP_killsignal)
            {
                fprintf (DDSIP_outfile, "\nTermination signal received.\n");
                if (DDSIP_param->outlev)
                    fprintf (DDSIP_bb->moreoutfile, "\nTermination signal received.\n");
                cb_destruct_problem (&p);
                DDSIP_Free ((void **) &(minfirst));
                DDSIP_Free ((void **) &(maxfirst));
                DDSIP_Free ((void **) &(center_point));
                return 0;
            }
            if (DDSIP_param->outlev > 7)
            {
                fprintf (DDSIP_bb->moreoutfile, "\nDescent step %d    next weight %g\n", DDSIP_bb->dualdescitcnt,next_weight);
            }

            if (DDSIP_param->outlev > 99)
            {
                // check the changes of the center point
                if ((status = cb_get_center (p, center_point)))
                {
                    printf ("get_center returned %d\n", status);
                }
                else
                {
                    fprintf (DDSIP_bb->moreoutfile, " --------------- current center point ---------------\n");
                    for (status=0; status < DDSIP_bb->dimdual; status++)
                    {
                        fprintf (DDSIP_bb->moreoutfile, " %14.8g,", center_point[status]);
                        if (!((status+1)%10))
                            fprintf (DDSIP_bb->moreoutfile, "\n");
                    }
                    fprintf (DDSIP_bb->moreoutfile, "\n ------------------------------\n");
                }
            }
            DDSIP_bb->dualObjVal = -DDSIP_infty;
            /* Make a descent step */
            /* status = cb_do_descent_step (p); */
            //int maxsteps = 10; /* DLW Dec 2014 - limit the number of null steps in order to avoid loops of reevaluations */
            cb_status = cb_do_maxsteps(p, DDSIP_param->cb_maxsteps + (DDSIP_bb->dualdescitcnt==1?5:0)); /* DLW Dec 2014 */
            // update dual solution
            cb_get_center (p,DDSIP_node[DDSIP_bb->curnode]->dual);
            DDSIP_node[DDSIP_bb->curnode]->dual[DDSIP_bb->dimdual] = cb_get_last_weight(p);
            if (cb_status == 111)
            {
                // Killsignal
                fprintf (DDSIP_outfile, "\nTermination signal received.\n");
                if (DDSIP_param->outlev)
                    fprintf (DDSIP_bb->moreoutfile, "\nTermination signal received.\n");
                cb_destruct_problem (&p);
                DDSIP_Free ((void **) &(minfirst));
                DDSIP_Free ((void **) &(maxfirst));
                DDSIP_Free ((void **) &(center_point));
                return 111;
            }
            else if (cb_status)
            {
                printf ("cb_do_descent_step returned %d\n", cb_status);
                if (DDSIP_param->outlev)
                    fprintf (DDSIP_bb->moreoutfile, "cb_do_descent_step returned %d\n", cb_status);
                cb_status = cb_termination_code(p);
                printf ("cb_termination_code returned %d\n", cb_status);
                if (DDSIP_param->outlev)
                    fprintf (DDSIP_bb->moreoutfile, "cb_termination_code returned %d\n", cb_status);
                cb_destruct_problem (&p);
                // Reset first stage solutions to the ones that gave the best bound
                for (j = 0; j < DDSIP_param->scenarios; j++)
                {
                    if (DDSIP_bb->bestfirst[j].first_sol)
                    {
                        if (((DDSIP_node[DDSIP_bb->curnode])->first_sol)[j])
                        {
                            if ((cnt = (((DDSIP_node[DDSIP_bb->curnode])->first_sol)[j])[DDSIP_bb->firstvar] - 1))
                                for (i_scen = j + 1; cnt && i_scen < DDSIP_param->scenarios; i_scen++)
                                {
                                    if ((((DDSIP_node[DDSIP_bb->curnode])->first_sol)[j]) == (((DDSIP_node[DDSIP_bb->curnode])->first_sol)[i_scen]))
                                    {
                                        ((DDSIP_node[DDSIP_bb->curnode])->first_sol)[i_scen] = NULL;
                                        cnt--;
                                    }
                                }
                            DDSIP_Free ((void **) &(((DDSIP_node[DDSIP_bb->curnode])->first_sol)[j]));
                        }
                        DDSIP_node[DDSIP_bb->curnode]->first_sol[j]   = (DDSIP_bb->bestfirst[j]).first_sol;
                        (DDSIP_node[DDSIP_bb->curnode]->cursubsol)[j] = (DDSIP_bb->bestfirst[j]).cursubsol;
                        (DDSIP_node[DDSIP_bb->curnode]->subbound)[j]  = (DDSIP_bb->bestfirst[j]).subbound;
                        DDSIP_bb->bestfirst[j].first_sol = NULL;
                    }
                }
                DDSIP_Free ((void **) &(minfirst));
                DDSIP_Free ((void **) &(maxfirst));
                DDSIP_Free ((void **) &(center_point));
                return 101;
            }

            if ((cb_status = cb_termination_code(p)))
            {
                printf ("cb_termination_code: %d\n", cb_status);
                if (DDSIP_param->outlev)
                    fprintf (DDSIP_bb->moreoutfile, "cb_termination_code: %d\n", cb_status);
            }
            /* Get solution information */
            // obj = -cb_get_objval (p);
            // take the best obj value in case the descent step was interrupted by maxsteps or other cause
            obj = DDSIP_bb->dualObjVal;
            next_weight = cb_get_last_weight (p);
            // if the step did not increase the bound, increase the weight
            if (!DDSIP_param->cb_inherit || DDSIP_bb->dualdescitcnt > 1|| (DDSIP_bb->dualdescitcnt == 1 && DDSIP_bb->dualitcnt < 4))
            {
                if (obj <= old_obj /*- 1.e-14*fabs(old_obj)*/)
                {
                    repeated_increase = 0;
                    noIncreaseCounter++;
                    many_iters +=2;
/////////
                    if (DDSIP_param->outlev > 10)
                    {
                        fprintf (DDSIP_bb->moreoutfile,"§§§§§§§§§§§§§§§ dualdescitcnt = %d, noIncreaseCounter= %d, last_weight= %g, current weight= %g, dualitcnt= %d §§§§§§§§§§§§§§§\n",DDSIP_bb->dualdescitcnt, noIncreaseCounter, last_weight, next_weight, DDSIP_bb->dualitcnt);
                    }
/////////
                    if (DDSIP_param->cb_increaseWeight)
                    {
                        if (noIncreaseCounter == 1)
                        {
                            if (next_weight < 1.1*last_weight)
                            {
                                last_weight = next_weight;
                                if (abs(DDSIP_param->riskmod) == 4)
                                    next_weight = 4.0*last_weight;
                                else if (abs(DDSIP_param->riskmod) == 5)
                                    next_weight = 1.8*last_weight;
                                else
                                {
                                    if (many_iters < 5)
                                        next_weight = 1.3*last_weight;
                                    else
                                        next_weight = 4.0*last_weight;
                                }
                                if (DDSIP_bb->dualdescitcnt == 1)
                                    next_weight = 1. + 10.*next_weight;
                                cb_set_next_weight (p, next_weight);
                                repeated_increase = -1;
                                if (DDSIP_param->outlev)
                                    fprintf (DDSIP_bb->moreoutfile,"####### §§§ increased next weight to %g\n", next_weight);
                            }
                        }
                        else if (noIncreaseCounter > 1)
                        {
                            last_weight = next_weight;
                            if (noIncreaseCounter > 3)
                            {
                                if (last_weight > 2000.)
                                {
                                    next_weight = 0.002*last_weight;
                                }
                                else
                                {
                                    next_weight = 100.*last_weight;
                                }
                                noIncreaseCounter = 1;
                            }
                            else if (last_weight < 50.)
                            {
                                if (DDSIP_bb->dualdescitcnt < 3 && DDSIP_bb->dualitcnt > DDSIP_bb->dualdescitcnt*DDSIP_param->cb_maxsteps + 3)
                                {
                                    if (last_weight < 2.)
                                    {
                                        next_weight = 1. + 20.*last_weight;
                                        if(DDSIP_param->outlev > 10)
                                            fprintf(DDSIP_bb->moreoutfile, "  next_weight=  1. + %g * %g = %g\n", 20., last_weight, next_weight);
                                    }
                                    else
                                    {
                                        next_weight = 10.*last_weight;
                                        if(DDSIP_param->outlev > 10)
                                            fprintf(DDSIP_bb->moreoutfile, "  next_weight= %g * %g = %g\n", 10., last_weight, next_weight);
                                    }
                                }
                                else
                                {
                                    cpu_secs=(old_obj - obj)/(fabs(old_obj)+1e-6);
                                    if(DDSIP_param->outlev > 10)
                                        fprintf(DDSIP_bb->moreoutfile, " relative decrease = %-16.12g",cpu_secs);
                                    if (cpu_secs < 6e-4)
                                    {
                                        next_weight = (1.3+1e3*cpu_secs)*last_weight;
                                        if(DDSIP_param->outlev > 10)
                                            fprintf(DDSIP_bb->moreoutfile, "  next_weight= %g * %g = %g\n", 1.3+1e3*cpu_secs, last_weight, next_weight);
                                    }
                                    else
                                    {
                                        next_weight = 4.0*last_weight;
                                        if(DDSIP_param->outlev > 10)
                                            fprintf(DDSIP_bb->moreoutfile, "  next_weight= %g * %g = %g\n", 4., last_weight, next_weight);
                                    }
                                }
                                repeated_increase = -1;
                            }
                            else
                            {
                                if (last_weight < 1.e4)
                                {
                                    if (many_iters < 4)
                                        next_weight = 4.0*last_weight;
                                    else
                                        next_weight = 8.0*last_weight;
                                }
                                else
                                    next_weight = 1.2*last_weight;
                            }
                            cb_set_next_weight (p, next_weight);
                            /////////
                            if (DDSIP_param->outlev)
                            {
                                fprintf (DDSIP_bb->moreoutfile,"####### §§§ increased next weight to %g\n", next_weight);
                            }
                            /////////
                            if (DDSIP_bb->dualdescitcnt == 1)
                                old_obj = obj;
                        }
                    }
//
                    if (DDSIP_param->outlev)
                    {
                        fprintf (DDSIP_bb->moreoutfile, " descent step %d: obj=%g, bound=%g, test for worse bound: %d\n", DDSIP_bb->dualdescitcnt, obj, DDSIP_node[DDSIP_bb->curnode]->bound,  (obj < DDSIP_node[DDSIP_bb->curnode]->bound - 1.e-4*fabs(DDSIP_node[DDSIP_bb->curnode]->bound)));
                    }
//
                    if (DDSIP_param->cb_changetol && !limits_reset && (obj < DDSIP_node[DDSIP_bb->curnode]->bound - 1.e-4*fabs(DDSIP_node[DDSIP_bb->curnode]->bound)))
                    {
                        // the branched problems produced a bound worse than already known - reset temporarily cplex tolerance and time limit
                        limits_reset = 2;
                        printf(" reset CPLEX relgap and/or time limit.\n");
                        if (DDSIP_param->outlev)
                            fprintf(DDSIP_bb->moreoutfile, " reset CPLEX relgap and/or time limit.\n");
                        for (cpu_hrs=0; cpu_hrs < DDSIP_param->cpxnodual; cpu_hrs++)
                        {
                            if (DDSIP_param->cpxdualwhich[cpu_hrs] == CPX_PARAM_TILIM)
                            {
                                old_cpxtimelim = DDSIP_param->cpxdualwhat[cpu_hrs];
                                DDSIP_param->cpxdualwhat[cpu_hrs] = 1.2*old_cpxtimelim;
                                status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                                if (status)
                                {
                                    printf("Failed to reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                }
                                else
                                {
                                    printf("temporarily reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                    if (DDSIP_param->outlev)
                                        fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                }
                            }
                            if (DDSIP_param->cpxdualwhich[cpu_hrs] == CPX_PARAM_EPGAP)
                            {
                                old_cpxrelgap = DDSIP_param->cpxdualwhat[cpu_hrs];
                                DDSIP_param->cpxdualwhat[cpu_hrs] = 0.20*old_cpxrelgap;
                                status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                                if (status)
                                {
                                    printf("Failed to reset CPLEX relgap to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                }
                                else
                                {
                                    printf("temporarily reset CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                    if (DDSIP_param->outlev)
                                        fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                }
                            }
                        }
                        for (cpu_hrs=0; cpu_hrs < DDSIP_param->cpxnodual2; cpu_hrs++)
                        {
                            if (DDSIP_param->cpxdualwhich2[cpu_hrs] == CPX_PARAM_TILIM)
                            {
                                old_cpxtimelim2 = DDSIP_param->cpxdualwhat2[cpu_hrs];
                                DDSIP_param->cpxdualwhat2[cpu_hrs] = 1.5*old_cpxtimelim2;
                                status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                if (status)
                                {
                                    printf("Failed to reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                }
                                else
                                {
                                    printf("temporarily reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                    if (DDSIP_param->outlev)
                                        fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                }
                            }
                            if (DDSIP_param->cpxdualwhich2[cpu_hrs] == CPX_PARAM_EPGAP)
                            {
                                old_cpxrelgap2 = DDSIP_param->cpxdualwhat2[cpu_hrs];
                                DDSIP_param->cpxdualwhat2[cpu_hrs] = 0.30*old_cpxrelgap2;
                                status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                if (status)
                                {
                                    printf("Failed to reset CPLEX relgap to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                }
                                else
                                {
                                    printf("temporarily reset CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                    if (DDSIP_param->outlev)
                                        fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                }
                            }
                        } // end for
                    }
                    if (!cb_status && !limits_reset && (noIncreaseCounter > 1 || (obj < DDSIP_node[DDSIP_bb->curnode]->bound - 1.e-4*fabs(DDSIP_node[DDSIP_bb->curnode]->bound))))
                    {
                        limits_reset = 1;
                        if (DDSIP_param->cb_changetol)
                        {
                            // the branched problems produced a worse bound than known - reset temporarily cplex tolerance and time limit
                            printf("reset CPLEX relgap and/or time limit.\n");
                            if (DDSIP_param->outlev)
                                fprintf(DDSIP_bb->moreoutfile, " reset CPLEX relgap and/or time limit.\n");
                            for (cpu_hrs=0; cpu_hrs < DDSIP_param->cpxnodual; cpu_hrs++)
                            {
                                if (DDSIP_param->cpxdualwhich[cpu_hrs] == CPX_PARAM_TILIM)
                                {
                                    old_cpxtimelim = DDSIP_param->cpxdualwhat[cpu_hrs];
                                    DDSIP_param->cpxdualwhat[cpu_hrs] = 1.2*old_cpxtimelim;
                                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                                    if (status)
                                    {
                                        printf("Failed to reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                    }
                                    else
                                    {
                                        printf("temporarily reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                        if (DDSIP_param->outlev)
                                            fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                    }
                                }
                                if (DDSIP_param->cpxdualwhich[cpu_hrs] == CPX_PARAM_EPGAP)
                                {
                                    old_cpxrelgap = DDSIP_param->cpxdualwhat[cpu_hrs];
                                    DDSIP_param->cpxdualwhat[cpu_hrs] = 0.20*old_cpxrelgap;
                                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                                    if (status)
                                    {
                                        printf("Failed to reset CPLEX relgap to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                    }
                                    else
                                    {
                                        printf("temporarily reset CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                        if (DDSIP_param->outlev)
                                            fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                    }
                                }
                            } // end for
                            for (cpu_hrs=0; cpu_hrs < DDSIP_param->cpxnodual2; cpu_hrs++)
                            {
                                if (DDSIP_param->cpxdualwhich2[cpu_hrs] == CPX_PARAM_TILIM)
                                {
                                    old_cpxtimelim2 = DDSIP_param->cpxdualwhat2[cpu_hrs];
                                    DDSIP_param->cpxdualwhat2[cpu_hrs] = 1.5*old_cpxtimelim2;
                                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                    if (status)
                                    {
                                        printf("Failed to reset CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                    }
                                    else
                                    {
                                        printf("temporarily reset CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                        if (DDSIP_param->outlev)
                                            fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                    }
                                }
                                if (DDSIP_param->cpxdualwhich2[cpu_hrs] == CPX_PARAM_EPGAP)
                                {
                                    old_cpxrelgap2 = DDSIP_param->cpxdualwhat2[cpu_hrs];
                                    DDSIP_param->cpxdualwhat2[cpu_hrs] = 0.30*old_cpxrelgap2;
                                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                    if (status)
                                    {
                                        printf("Failed to reset CPLEX relgap 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                    }
                                    else
                                    {
                                        printf("temporarily reset CPLEX relgap tolerance 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                        if (DDSIP_param->outlev)
                                            fprintf(DDSIP_bb->moreoutfile, "temporarily reset CPLEX relgap tolerance 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                    }
                                }
                            } // end for
                        }
                    }
//
                    if(DDSIP_param->outlev > 20)
                        fprintf(DDSIP_bb->moreoutfile, " obj -  old_obj = %-16.12g - %-16.12g = %-10.6g, \tnoIncreaseCounter= %d, last_weight= %g, next_weight= %g\n",obj, old_obj, obj - old_obj, noIncreaseCounter, last_weight, next_weight);
//
                    // the evaluation in bestdual might have increased the bound
                    obj = DDSIP_bb->dualObjVal;
                }
                else
                {
                    noIncreaseCounter = 0;
                    next_weight = cb_get_last_weight (p);
///////////////////////
                    if (DDSIP_param->outlev > 10)
                        fprintf(DDSIP_bb->moreoutfile,"############### iters in descent step: %d,  repeated increase up to now: %d, last_weight - next_weight= %g ##################\n", DDSIP_bb->dualitcnt - last_dualitcnt,repeated_increase,  last_weight - next_weight);
/////////
                    if (DDSIP_bb->dualitcnt - last_dualitcnt < 4 && last_weight >= DDSIP_Dmin(0.5,reduction_factor)*next_weight)
                    {
                        many_iters = 0;
                        repeated_increase += 4 - DDSIP_bb->dualitcnt + last_dualitcnt;
                        if (DDSIP_param->cb_reduceWeight && next_weight > 5.e-5)
                        {
                            if (repeated_increase > 2)
                            {
                                if (weight_decreases > 1)
                                {
                                    reduction_factor = 0.75*reduction_factor + 0.025;
                                    weight_decreases = 1;
///////////////////////
                                    if (DDSIP_param->outlev > 10)
                                        fprintf(DDSIP_bb->moreoutfile,"############### repeated increase= %d, reduction factor decreased to %g ##################\n",repeated_increase, reduction_factor);
///////////////////////
                                }
                                else
                                    weight_decreases++;
                                last_weight = next_weight;
                                next_weight = last_weight * reduction_factor;
                                cb_set_next_weight (p, next_weight);
///////////////////////
                                if (DDSIP_param->outlev > 10)
                                    fprintf(DDSIP_bb->moreoutfile,"############### reduced next weight to %g,  repeated increase= %d ##################\n",next_weight,repeated_increase);
///////////////////////
                                //repeated_increase = 1;
                                repeated_increase = 0;
                            }
                        }
                    }
                    else if (DDSIP_param->cb_increaseWeight && DDSIP_bb->dualdescitcnt > 1 && last_weight*10. >= next_weight)
                    {
                        if ((j = DDSIP_bb->dualitcnt - last_dualitcnt) > 4)
                        {
                            repeated_increase = 0;
                            if (j > 5)
                            {
                                repeated_increase = -1;
                                many_iters++;
                                if (weight_decreases)
                                {
                                    reduction_factor = 0.7*reduction_factor + 0.27;
                                    weight_decreases = 0;
///////////////////////
                                    if (DDSIP_param->outlev > 10)
                                        fprintf(DDSIP_bb->moreoutfile,"############### repeated increase= %d, reduction factor increased to %g ##################\n",repeated_increase, reduction_factor);
///////////////////////
                                }
                                if (j > 6)
                                {
                                    repeated_increase = -2;
                                    if (j > 9)
                                    {
                                        if (next_weight < 1.10*last_weight)
                                        {
                                            last_weight = next_weight;
                                            if (last_weight > 1.)
                                                next_weight = last_weight * 1.05;
                                            else if (last_weight > 1e-2)
                                                next_weight = last_weight * 1.20;
                                            else
                                                next_weight = last_weight * 2.50;
                                            cb_set_next_weight (p, next_weight);
///////////////////////
                                            if (DDSIP_param->outlev > 10)
                                                fprintf(DDSIP_bb->moreoutfile,"############### increased next weight to %g,  repeated increase= %d ##################\n",next_weight,repeated_increase);
///////////////////////
                                        }
                                        many_iters++;
                                    }
                                }
                            }
                        }
                        //if (DDSIP_param->cb_reduceWeight && many_iters > 4)
                        if (many_iters > 4)
                        {
                            next_weight = next_weight * 1.15;
                            cb_set_next_weight (p, next_weight);
///////////////////////
                            if (DDSIP_param->outlev > 10)
                                fprintf(DDSIP_bb->moreoutfile,"############### many_iters=%d-> too many iters, increased next weight to %g,  repeated increase= %d ##################\n",many_iters,next_weight,repeated_increase);
///////////////////////
                            many_iters = 0;
                        }
                    }
///////////////////////
                    if (DDSIP_param->outlev > 10)
                        fprintf(DDSIP_bb->moreoutfile,"############### repeated_increase= %d  many_iters= %d    reduction_factor= %g ##################\n",repeated_increase,many_iters, reduction_factor);
///////////////////////
//
                    if(DDSIP_param->outlev > 10)
                        fprintf(DDSIP_bb->moreoutfile, " obj -  old_obj = %-16.12g - %-16.12g = %-10.6g, \tnoIncreaseCounter= %d, last_weight= %g, next_weight= %g\n",obj, old_obj, obj - old_obj, noIncreaseCounter, last_weight, next_weight);
//
                    old_obj = obj;

                }
            }
            else
            {
                old_obj = DDSIP_bb->dualObjVal;
            }

            last_weight = cb_get_last_weight (p);


            if (limits_reset && obj > DDSIP_node[DDSIP_bb->curnode]->bound - 1.e-5)
            {
                limits_reset = 0;
                if (DDSIP_param->cb_changetol)
                {
                    // revert to original cplex tolerance and time limit
                    for (cpu_hrs=0; cpu_hrs < DDSIP_param->cpxnodual; cpu_hrs++)
                    {
                        if (DDSIP_param->cpxdualwhich[cpu_hrs] == CPX_PARAM_TILIM)
                        {
                            DDSIP_param->cpxdualwhat[cpu_hrs] = old_cpxtimelim;
                            status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                            if (status)
                            {
                                printf("Failed to revert CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                            }
                            else
                            {
                                printf("Reverted CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                if (DDSIP_param->outlev)
                                    fprintf(DDSIP_bb->moreoutfile, "Reverted CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                            }
                        }
                        if (DDSIP_param->cpxdualwhich[cpu_hrs] == CPX_PARAM_EPGAP)
                        {
                            DDSIP_param->cpxdualwhat[cpu_hrs] = old_cpxrelgap;
                            status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                            if (status)
                            {
                                printf("Failed to revert CPLEX relgap to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                            }
                            else
                            {
                                printf("Reverted CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                                if (DDSIP_param->outlev)
                                    fprintf(DDSIP_bb->moreoutfile, "Reverted CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                            }
                        }
                    } // end for
                    for (cpu_hrs=0; cpu_hrs < DDSIP_param->cpxnodual2; cpu_hrs++)
                    {
                        if (DDSIP_param->cpxdualwhich2[cpu_hrs] == CPX_PARAM_TILIM)
                        {
                            DDSIP_param->cpxdualwhat2[cpu_hrs] = old_cpxtimelim2;
                            status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                            if (status)
                            {
                                printf("Failed to revert CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                            }
                            else
                            {
                                printf("Reverted CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                if (DDSIP_param->outlev)
                                    fprintf(DDSIP_bb->moreoutfile, "Reverted CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                            }
                        }
                        if (DDSIP_param->cpxdualwhich2[cpu_hrs] == CPX_PARAM_EPGAP)
                        {
                            DDSIP_param->cpxdualwhat2[cpu_hrs] = old_cpxrelgap2;
                            status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                            if (status)
                            {
                                printf("Failed to revert CPLEX relgap 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                            }
                            else
                            {
                                printf("Reverted CPLEX relgap tolerance 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                                if (DDSIP_param->outlev)
                                    fprintf(DDSIP_bb->moreoutfile, "Reverted CPLEX relgap tolerance 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                            }
                        }
                    }
                }
            }
            // Print iteration info
            if (DDSIP_param->outlev)
            {
                DDSIP_translate_time (DDSIP_GetCpuTime(),&cpu_hrs,&cpu_mins,&cpu_secs);
                printf ("  | %23d  %7d  %-16.12g %-11.6g   %-16.12g%20dh %02d:%05.2f\n",
                        DDSIP_bb->dualdescitcnt, DDSIP_bb->dualitcnt, obj, last_weight, DDSIP_node[DDSIP_bb->curnode]->bound, cpu_hrs,cpu_mins,cpu_secs);
                fprintf (DDSIP_outfile,
                         "  | %23d  %7d  %-16.12g %-11.6g   %-16.12g%20dh %02d:%05.2f\n",
                         DDSIP_bb->dualdescitcnt, DDSIP_bb->dualitcnt, obj, last_weight, DDSIP_node[DDSIP_bb->curnode]->bound, cpu_hrs,cpu_mins,cpu_secs);

            }
            last_dualitcnt = DDSIP_bb->dualitcnt;
            last_weight = next_weight;
        }
        if (DDSIP_param->outlev)
        {
            printf ("   --------- ");
            fprintf (DDSIP_outfile, "   --------- ");
            i_scen=cb_termination_code (p);
            if ((obj - DDSIP_bb->bestvalue)/(fabs(DDSIP_bb->bestvalue)+1.e-10) > DDSIP_param->accuracy)
            {
                DDSIP_Print2 ("termination status: cutoff.", "\n", 0, 0);
                DDSIP_bb->skip = 2;
                DDSIP_bb->cutoff++;
            }
            else if (!DDSIP_bb->violations)
                DDSIP_Print2 ("termination status: optimal.", "\n", 0, 0);
            else if (!i_scen)
            {
                if (DDSIP_bb->curnode && (DDSIP_bb->dualdescitcnt >= DDSIP_param->cbitlim ||  (DDSIP_bb->curnode < DDSIP_param->cbBreakIters && DDSIP_bb->dualdescitcnt >= (DDSIP_param->cbitlim+1)/2)))
                    DDSIP_Print2 ("termination status: descent iteration limit exceeded.", "\n", 0, 0);
                else if (!DDSIP_bb->curnode && DDSIP_bb->dualdescitcnt >= DDSIP_param->cbrootitlim )
                    DDSIP_Print2 ("termination status: descent iteration limit exceeded.", "\n", 0, 0);
                else if (DDSIP_bb->dualitcnt >= DDSIP_param->cbtotalitlim)
                    DDSIP_Print2 ("termination status: total iteration limit exceeded.", "\n", 0, 0);
                else if (noIncreaseCounter >= 9)
                    DDSIP_Print2 ("termination status: no bound increase achieved.", "\n", 0, 0);
                else if (DDSIP_GetCpuTime () > DDSIP_param->timelim)
                    DDSIP_Print2 ("Time limit reached.", "\n", 0, 0);
                else
                    fprintf (DDSIP_outfile, " Unidentified reason for stopping. Termination code of ConicBundle: %d\n", i_scen);
            }
            else
            {
                cb_print_termination_code (p);
                fprintf (DDSIP_outfile, " termination code of ConicBundle: %d ", (i_scen));
                if (i_scen == 1)
                    fprintf (DDSIP_outfile, "Relative precision criterion satisfied.\n");
                else if (i_scen == 2)
                    fprintf (DDSIP_outfile, "Time limit exceeded.\n");
                else if (i_scen == 4)
                    fprintf (DDSIP_outfile, "Maximum number of function reevaluations exceeded.\n");
                else if (i_scen == 8)
                    fprintf (DDSIP_outfile, "Maximum number of quadratic subproblem failures exceeded.\n");
                else if (i_scen == 16)
                    fprintf (DDSIP_outfile, "Maximum number of model evaluation failures exceeded.\n");
                else if (i_scen == 32)
                {
                    fprintf (DDSIP_outfile, "Maximum number of failures to increase the augmented model value exceeded.\n");
                    DDSIP_node[DDSIP_bb->curnode]->bestdual[DDSIP_bb->dimdual] = DDSIP_node[DDSIP_bb->curnode]->dual[DDSIP_bb->dimdual] = cb_get_last_weight(p);
                    if (DDSIP_param->outlev > 19)
                    {
                        fprintf (DDSIP_bb->moreoutfile, "\nMaximum number of failures to increase the augmented model value exceeded in node %d:   changed weight in bestdual to last weight= %g\n", DDSIP_bb->curnode, DDSIP_node[DDSIP_bb->curnode]->bestdual[DDSIP_bb->dimdual]);
                    }
                }
                else if (DDSIP_GetCpuTime () > DDSIP_param->timelim)
                    fprintf (DDSIP_outfile, "Total time limit exceeded.\n");
                else
                    fprintf (DDSIP_outfile, "\n");
            }
        }
        if (!(DDSIP_bb->curnode))
        {
            //printf ("   --------- sorting scenarios ---------\n");
            // in order to allow for premature cutoff: sort scenarios according to lower bound in root node in descending order
            double * sort_array;
            sort_array = (double *) DDSIP_Alloc(sizeof(double), DDSIP_param->scenarios, "sort_array(LowerBound)");
            for (i_scen = 0; i_scen < DDSIP_param->scenarios; i_scen++)
            {
                sort_array[i_scen] = DDSIP_data->prob[i_scen] * (DDSIP_node[DDSIP_bb->curnode]->subbound)[i_scen];
            }
            DDSIP_qsort_ins_D (sort_array, DDSIP_bb->lb_scen_order, 0, DDSIP_param->scenarios-1);
            DDSIP_Free ((void**) &sort_array);

            // initially: sort the scenarios for upper bounds in the same oder. This might be useful to stop evaluation prematurely.
            // This order is changed as soon as a suggested heuristics points is infeasible for one of the scenarios
            for (i_scen=0; i_scen<DDSIP_param->scenarios; i_scen++)
            {
                DDSIP_bb->scen_order[i_scen] = DDSIP_bb->lb_scen_order[i_scen];
            }
        }
    }

    // update dual solution
    //cb_get_center (p,DDSIP_node[DDSIP_bb->curnode]->dual);
    memcpy (DDSIP_node[DDSIP_bb->curnode]->dual, DDSIP_node[DDSIP_bb->curnode]->bestdual, sizeof (double) * DDSIP_bb->dimdual + 1);
    //DDSIP_node[DDSIP_bb->curnode]->dual[DDSIP_bb->dimdual] = cb_get_last_weight(p);
#ifdef MDEBUG
////////////////////////////////////////////
    if(DDSIP_param->outlev)
        fprintf (DDSIP_bb->moreoutfile," ******************** node %d has dual %p with weight %g\n", DDSIP_bb->curnode, DDSIP_node[DDSIP_bb->curnode]->dual, DDSIP_node[DDSIP_bb->curnode]->dual[DDSIP_bb->dimdual]);
////////////////////////////////////////////
#endif

    if (DDSIP_param->outlev > 19)
    {
        fprintf (DDSIP_bb->moreoutfile, "\nFinal lambda for node %d:   weight in bestdual= %g, last weight= %g\n MULTIPLIER\n", DDSIP_bb->curnode, DDSIP_node[DDSIP_bb->curnode]->bestdual[DDSIP_bb->dimdual], DDSIP_node[DDSIP_bb->curnode]->dual[DDSIP_bb->dimdual]);
        for (j = 0; j < DDSIP_bb->dimdual; j++)
        {
            fprintf (DDSIP_bb->moreoutfile, "  %14.8g", DDSIP_node[DDSIP_bb->curnode]->bestdual[j]);
            if (!((j+1)%10))
                fprintf (DDSIP_bb->moreoutfile, "\n");
        }
        fprintf (DDSIP_bb->moreoutfile, "\n");
    }

    cb_destruct_problem (&p);
    for (j = 0; j < DDSIP_bb->firstvar; j++)
    {
        minfirst[j] = DDSIP_infty;
        maxfirst[j] = -DDSIP_infty;
    }
    // Reset first stage solutions to the ones that gave the best bound
    for (j = 0; j < DDSIP_param->scenarios; j++)
    {
        if (DDSIP_bb->bestfirst[j].first_sol)
        {
            if (((DDSIP_node[DDSIP_bb->curnode])->first_sol)[j])
            {
                if ((cnt = (((DDSIP_node[DDSIP_bb->curnode])->first_sol)[j])[DDSIP_bb->firstvar] - 1))
                    for (i_scen = j + 1; cnt && i_scen < DDSIP_param->scenarios; i_scen++)
                    {
                        if ((((DDSIP_node[DDSIP_bb->curnode])->first_sol)[j]) == (((DDSIP_node[DDSIP_bb->curnode])->first_sol)[i_scen]))
                        {
                            ((DDSIP_node[DDSIP_bb->curnode])->first_sol)[i_scen] = NULL;
                            cnt--;
                        }
                    }
                DDSIP_Free ((void **) &(((DDSIP_node[DDSIP_bb->curnode])->first_sol)[j]));
            }
            DDSIP_node[DDSIP_bb->curnode]->first_sol[j]   = (DDSIP_bb->bestfirst[j]).first_sol;
            (DDSIP_node[DDSIP_bb->curnode]->cursubsol)[j] = (DDSIP_bb->bestfirst[j]).cursubsol;
            (DDSIP_node[DDSIP_bb->curnode]->subbound)[j]  = (DDSIP_bb->bestfirst[j]).subbound;
            DDSIP_bb->bestfirst[j].first_sol = NULL;
            for (cnt=0; cnt<DDSIP_bb->firstvar; cnt++)
            {
                minfirst[cnt]=DDSIP_Dmin (minfirst[cnt],DDSIP_node[DDSIP_bb->curnode]->first_sol[j][cnt]);
                maxfirst[cnt]=DDSIP_Dmax (maxfirst[cnt],DDSIP_node[DDSIP_bb->curnode]->first_sol[j][cnt]);
            }
        }
    }
    // revert changes to  cplex tolerance and time limit
    if (limits_reset)
    {
        // revert to original cplex tolerance and time limit
        limits_reset = 0;
        if (DDSIP_param->cb_changetol)
        {
            for (cpu_hrs=0; cpu_hrs < DDSIP_param->cpxnodual; cpu_hrs++)
            {
                if (DDSIP_param->cpxdualwhich[cpu_hrs] == CPX_PARAM_TILIM)
                {
                    DDSIP_param->cpxdualwhat[cpu_hrs] = old_cpxtimelim;
                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                    if (status)
                    {
                        printf("Failed to revert CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                    }
                    else
                    {
                        printf("Reverted CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                        if (DDSIP_param->outlev)
                            fprintf(DDSIP_bb->moreoutfile, "Reverted CPLEX time limit to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                    }
                }
                if (DDSIP_param->cpxdualwhich[cpu_hrs] == CPX_PARAM_EPGAP)
                {
                    DDSIP_param->cpxdualwhat[cpu_hrs] = old_cpxrelgap;
                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich[cpu_hrs], DDSIP_param->cpxdualwhat[cpu_hrs]);
                    if (status)
                    {
                        printf("Failed to revert CPLEX relgap to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                    }
                    else
                    {
                        printf("Reverted CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                        if (DDSIP_param->outlev)
                            fprintf(DDSIP_bb->moreoutfile, "Reverted CPLEX relgap tolerance to %g.\n", DDSIP_param->cpxdualwhat[cpu_hrs]);
                    }
                }
            } // end for
            for (cpu_hrs=0; cpu_hrs < DDSIP_param->cpxnodual2; cpu_hrs++)
            {
                if (DDSIP_param->cpxdualwhich2[cpu_hrs] == CPX_PARAM_TILIM)
                {
                    DDSIP_param->cpxdualwhat2[cpu_hrs] = old_cpxtimelim2;
                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    if (status)
                    {
                        printf("Failed to revert CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    }
                    else
                    {
                        printf("Reverted CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                        if (DDSIP_param->outlev)
                            fprintf(DDSIP_bb->moreoutfile, "Reverted CPLEX time limit 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    }
                }
                if (DDSIP_param->cpxdualwhich2[cpu_hrs] == CPX_PARAM_EPGAP)
                {
                    DDSIP_param->cpxdualwhat2[cpu_hrs] = old_cpxrelgap2;
                    status = CPXsetdblparam (DDSIP_env, DDSIP_param->cpxdualwhich2[cpu_hrs], DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    if (status)
                    {
                        printf("Failed to revert CPLEX relgap 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    }
                    else
                    {
                        printf("Reverted CPLEX relgap tolerance 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                        if (DDSIP_param->outlev)
                            fprintf(DDSIP_bb->moreoutfile, "Reverted CPLEX relgap tolerance 2nd opt. to %g.\n", DDSIP_param->cpxdualwhat2[cpu_hrs]);
                    }
                }
            }
        }
    }
    // Reset cplex parameters
    if (DDSIP_param->cpxnodual)
    {
        status = DDSIP_SetCpxPara (DDSIP_param->cpxno, DDSIP_param->cpxisdbl, DDSIP_param->cpxwhich, DDSIP_param->cpxwhat);
        if (status)
        {
            fprintf (stderr, "ERROR: Failed to reset CPLEX parameters (DualOpt) \n");
            DDSIP_Free ((void **) &(minfirst));
            DDSIP_Free ((void **) &(maxfirst));
            DDSIP_Free ((void **) &(center_point));
            return status;
        }
    }
    //determine variable to branch on
    if (!DDSIP_node[DDSIP_bb->curnode]->leaf)
    {
        for (j = 0; j < DDSIP_bb->firstvar; j++)
        {
            maxfirst[j] -= minfirst[j];
            if (fabs(maxfirst[j])>DDSIP_param->nulldisp)
            {
                if (DDSIP_param->outlev>20)
                    fprintf (DDSIP_bb->moreoutfile," ---- Deviation of variable %d : %g\n",j,maxfirst[j]);
            }
        }
        status = DDSIP_GetBranchIndex (maxfirst);
    }
    DDSIP_Free ((void **) &(minfirst));
    DDSIP_Free ((void **) &(maxfirst));
    DDSIP_Free ((void **) &(center_point));
#ifdef MDEBUG
    if(DDSIP_param->outlev)
        fprintf (DDSIP_bb->moreoutfile, " * End of DualOpt *** node %d has dual %p with weight %g, bestdual weight %g\n", DDSIP_bb->curnode, DDSIP_node[DDSIP_bb->curnode]->dual, DDSIP_node[DDSIP_bb->curnode]->dual[DDSIP_bb->dimdual],  DDSIP_node[DDSIP_bb->curnode]->bestdual[DDSIP_bb->dimdual]);
#endif
    return 0;
}
#endif