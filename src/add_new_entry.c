/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   add_new_entry.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmondino <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/07/02 12:22:33 by jmondino          #+#    #+#             */
/*   Updated: 2019/07/02 14:29:04 by jmondino         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ls.h"

void fill_entry(t_entry *entry, struct stat pstat, int ut_flags)
{
    entry->type = pstat.st_mode;
    if (S_ISBLK(entry->type) || S_ISCHR(entry->type))
    {
        entry->minor = minor(pstat.st_rdev);
        entry->major = major(pstat.st_rdev);
    }
    entry->rights = permissions(pstat.st_mode);
    entry->hard_links = pstat.st_nlink;
    entry->size = pstat.st_size;
    entry->user = ft_strdup(getpwuid(pstat.st_uid) == NULL ? "" : getpwuid(pstat.st_uid)->pw_name);
    entry->group = ft_strdup(getgrgid(pstat.st_gid) == NULL ? "" : getgrgid(pstat.st_gid)->gr_name);
    entry->date_day_modified = get_day(ctime(ut_flags ? &pstat.st_atimespec.tv_sec : &pstat.st_mtimespec.tv_sec));
    entry->block_size = pstat.st_blocks;
    entry->date_month_modified =
        ft_strsub(ctime(ut_flags ? &pstat.st_atimespec.tv_sec : &pstat.st_mtimespec.tv_sec), 4, 3);
    entry->mtime = pstat.st_mtime;
    entry->date_time_modified = (time(0) - pstat.st_mtime) > SIXMONTHS ?
        ft_strsub(ctime(ut_flags ? &pstat.st_atimespec.tv_sec : &pstat.st_mtimespec.tv_sec), 20, 4) :
        ft_strsub(ctime(ut_flags ? &pstat.st_atimespec.tv_sec : &pstat.st_mtimespec.tv_sec), 11, 5);
    entry->date_accessed = pstat.st_atimespec.tv_sec;
    entry->next = NULL;
}

t_entry     *add_new_entry(char *path, char *entry_name, t_args *pargs)
{
    t_entry *entry;
    struct stat pstat;
    char  l[1024];
    int ut_flags;

    ut_flags = (ft_iscinstr(pargs->flags, 't') && ft_iscinstr(pargs->flags, 'u'));
    if ((entry = malloc(sizeof(t_entry))) == NULL)
        return 0;
    if (lstat(path, &pstat) == -1)
        return 0;
    entry->has_xattr = listxattr(path, l, 1024,  XATTR_NOFOLLOW);
    if(entry->has_xattr)
    {
        entry->xattr = get_xattr_list(l, entry->has_xattr);
        entry->xattr_sizes = get_xattr_sizes(entry->xattr, path, get_xattr_num(l, entry->has_xattr));
    }
    entry->has_acl = has_acl(path);
    entry->link_path = get_link_path(path);
    entry->name = ft_strdup(entry_name);
    fill_entry(entry, pstat, ut_flags);
    return (entry);
}

int     get_day(char *date)
{
    int i;

    i = 0;
    if (date)
    {
        while (!ft_isdigit(date[i]))
            i++;
        return (ft_atoi(date + i));
    }
    return 0;
}

char    *permissions(mode_t perm)
{
    char        *modeval;

    if((modeval = malloc(sizeof(char) * 9 + 1)) == NULL)
        return 0;
    modeval[0] = (perm & S_IRUSR) ? 'r' : '-';
    modeval[1] = (perm & S_IWUSR) ? 'w' : '-';
    modeval[2] = (perm & S_IXUSR) ? 'x' : '-';
    modeval[3] = (perm & S_IRGRP) ? 'r' : '-';
    modeval[4] = (perm & S_IWGRP) ? 'w' : '-';
    modeval[5] = (perm & S_IXGRP) ? 'x' : '-';
    modeval[6] = (perm & S_IROTH) ? 'r' : '-';
    modeval[7] = (perm & S_IWOTH) ? 'w' : '-';
    if ((perm & S_ISVTX))
        modeval[8] = 't';
    else
        modeval[8] = (perm & S_IXOTH) ? 'x' : '-';
    modeval[9] = '\0';
    return (modeval);
}

char    *get_link_path(char *path)
{
    char        *buf;
    ssize_t     len;

    if ((buf = malloc(1024)) == NULL)
        return (0);
    len = readlink(path, buf, 1023);
    if (len != -1)
    {
        buf[len] = '\0';
        return (buf);
    }
    else
    {
        free(buf);
        return (ft_strdup(""));
    }
}