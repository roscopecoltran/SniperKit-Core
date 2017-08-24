from sys import stdout

def print_action(action):
    def print_action_decorator(function):
        def puts(string):
            stdout.write(string)
            stdout.flush()

        def function_wrapper(*args, **kwargs):
            puts("{0}... ".format(action))
            return_value = function(*args, **kwargs)
            puts("Done!\n")
            return return_value

        return function_wrapper
    return print_action_decorator


def format_plot_axes(axes):
    axes.xaxis.set_ticks_position('bottom')
    axes.yaxis.set_ticks_position('none')
    axes.spines['top'].set_color('none')
    axes.spines['right'].set_color('none')
