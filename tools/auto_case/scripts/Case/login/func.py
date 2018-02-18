def check_alterface(cli):
    role = cli.user
    if None == role:
        return 'failed'
    if role.face == cli.user.old.face:
        return 'success'
    return 'failed'
